// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	PreviousRotation = BlasterCharacter ? BlasterCharacter->GetActorRotation() : FRotator::ZeroRotator;
	bWasTurningEligible = false;
	RootYawOffset = TurnRootYaw = TurnAnimationTime = 0.f;
	bTurningLeft = bTurningRight = false;
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	bLeftHandIKValid = false;
	LeftHandTransform = FTransform::Identity;

	if (!IsValid(BlasterCharacter))
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
		PreviousRotation = BlasterCharacter ? BlasterCharacter->GetActorRotation() : FRotator::ZeroRotator;
	}
	if (!IsValid(BlasterCharacter)) return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bAiming = BlasterCharacter->IsAiming();
	UpdateTurning(DeltaSeconds);

	// Each network instance derives IK from its replicated, attached weapon.
	// Invalid data disables the solver instead of targeting the component origin.
	AWeapon* EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	USkeletalMeshComponent* CharacterMesh = BlasterCharacter->GetMesh();
	USkeletalMeshComponent* WeaponMesh = IsValid(EquippedWeapon) ? EquippedWeapon->GetWeaponMesh() : nullptr;
	static const FName LeftHandSocketName(TEXT("LeftHandSocket"));
	static const FName RightHandBoneName(TEXT("hand_r"));
	static const FName LeftHandBoneName(TEXT("hand_l"));
	static const FName LeftArmBoneName(TEXT("upperarm_l"));
	if (IsValid(CharacterMesh) && IsValid(WeaponMesh)
		&& WeaponMesh->GetAttachParent() == CharacterMesh
		&& WeaponMesh->DoesSocketExist(LeftHandSocketName)
		&& CharacterMesh->GetBoneIndex(RightHandBoneName) != INDEX_NONE
		&& CharacterMesh->GetBoneIndex(LeftHandBoneName) != INDEX_NONE
		&& CharacterMesh->GetBoneIndex(LeftArmBoneName) != INDEX_NONE)
	{
		const FTransform SocketTransform = WeaponMesh->GetSocketTransform(LeftHandSocketName, RTS_World);
		FVector BoneSpaceLocation = FVector::ZeroVector;
		FRotator BoneSpaceRotation = FRotator::ZeroRotator;
		CharacterMesh->TransformToBoneSpace(RightHandBoneName, SocketTransform.GetLocation(),
			SocketTransform.Rotator(), BoneSpaceLocation, BoneSpaceRotation);
		LeftHandTransform = FTransform(BoneSpaceRotation, BoneSpaceLocation);
		bLeftHandIKValid = true;
	}

	const FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	AO_Pitch = AimRotation.Pitch;

	// Remote view pitch can arrive as a 270..360 degree value. Convert it to
	// the -90..0 range expected by the Aim Offset asset.
	if (!BlasterCharacter->IsLocallyControlled() && AO_Pitch > 90.f)
	{
		AO_Pitch = FMath::GetMappedRangeValueClamped(
			FVector2D(270.f, 360.f),
			FVector2D(-90.f, 0.f),
			AO_Pitch);
	}

	const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.f);
	const FVector MovementDirection = HorizontalVelocity.IsNearlyZero()
		? BlasterCharacter->GetActorForwardVector()
		: HorizontalVelocity.GetSafeNormal();
	const FRotator MovementRotation = MovementDirection.Rotation();
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	if (DeltaSeconds > KINDA_SMALL_NUMBER)
	{
		const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(
			BlasterCharacter->GetActorRotation(), PreviousRotation);
		const float TargetLean = FMath::Clamp(DeltaRotation.Yaw / DeltaSeconds, -90.f, 90.f);
		Lean = FMath::FInterpTo(Lean, TargetLean, DeltaSeconds, 6.f);
	}
	PreviousRotation = BlasterCharacter->GetActorRotation();
}

void UBlasterAnimInstance::UpdateTurning(float DeltaSeconds)
{
	const float ActorYaw = BlasterCharacter->GetActorRotation().Yaw;
	const bool bEligible = bWeaponEquipped && Speed < 3.f && !bIsInAir && !bIsCrouched
		&& IsValid(TurnLeftAnimation) && IsValid(TurnRightAnimation);
	if (!bEligible || !bWasTurningEligible)
	{
		RootYawOffset = TurnRootYaw = TurnAnimationTime = 0.f;
		bTurningLeft = bTurningRight = false;
		PreviousTurnYaw = ActorYaw;
		bWasTurningEligible = bEligible;
		return;
	}

	const float YawDelta = FMath::FindDeltaAngleDegrees(PreviousTurnYaw, ActorYaw);
	PreviousTurnYaw = ActorYaw;
	RootYawOffset = FMath::Clamp(FMath::UnwindDegrees(RootYawOffset - YawDelta), -135.f, 135.f);
	// Replicated yaw is quantized. A small tolerance keeps an exact quarter-turn
	// from triggering on one machine but waiting indefinitely on another.
	if (!bTurningLeft && !bTurningRight && FMath::Abs(RootYawOffset) >= 89.5f)
	{
		bTurningLeft = RootYawOffset > 0.f;
		bTurningRight = !bTurningLeft;
		TurnAnimationTime = 0.f;
		const UAnimSequence* Animation = bTurningLeft ? TurnLeftAnimation.Get() : TurnRightAnimation.Get();
		TurnRate = FMath::Abs(RootYawOffset) / FMath::Max(Animation->GetPlayLength(), 0.01f);
	}

	TurnRootYaw = RootYawOffset;
	if (bTurningLeft || bTurningRight)
	{
		const UAnimSequence* Animation = bTurningLeft ? TurnLeftAnimation.Get() : TurnRightAnimation.Get();
		const float SafeDelta = FMath::Max(DeltaSeconds, 0.f);
		TurnAnimationTime = FMath::Min(TurnAnimationTime + SafeDelta, Animation->GetPlayLength());
		RootYawOffset = FMath::FInterpConstantTo(RootYawOffset, 0.f, SafeDelta, TurnRate);
		// Remove the heading already baked into the source animation. Actor yaw
		// plus the remaining root offset supplies the visible turning instead.
		const float AnimatedYaw = Animation->EvaluateCurveData(TEXT("B01TurnYaw"),
			FAnimExtractContext(static_cast<double>(TurnAnimationTime)));
		TurnRootYaw = RootYawOffset - AnimatedYaw;
		if (TurnAnimationTime >= Animation->GetPlayLength())
		{
			bTurningLeft = bTurningRight = false;
			TurnAnimationTime = 0.f;
			TurnRootYaw = RootYawOffset;
		}
	}
}
