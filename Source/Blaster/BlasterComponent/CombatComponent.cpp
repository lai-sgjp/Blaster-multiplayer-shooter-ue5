#include "CombatComponent.h"
#include "Blaster/Character/BlasterPlayerController.h"

#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/BlasterProjectile.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Blaster/Weapon/BlasterCasing.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "LagCompensationComponent.h"
#include "Blaster/Weapon/BlasterShotEffect.h"
#include "BlasterHitRules.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarShotDebug(TEXT("blaster.DebugShots"), 0, TEXT("Draw aim and muzzle blockers"));

void UCombatComponent::ClientConfirmHit_Implementation(bool bHeadshot)
{
	HitFeedback = 0.25f;
	bLastHeadshot = bHeadshot;
	if (Character) if (auto* PC = Cast<ABlasterPlayerController>(Character->GetController())) PC->ClientFeedback(bHeadshot ? 1 : 0);
}

bool UCombatComponent::IsMuzzleBlocked(FHitResult& Hit) const
{
	if (!IsValid(Character) || !IsValid(EquippedWeapon)) return false;
	const FVector Muzzle = EquippedWeapon->GetWeaponMesh()->GetSocketLocation(TEXT("Muzzle"));
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BlasterClearance), false, Character);
	Params.AddIgnoredActor(EquippedWeapon);
	if (IsValid(SecondaryWeapon)) Params.AddIgnoredActor(SecondaryWeapon);
	return GetWorld()->LineTraceSingleByChannel(Hit, Character->GetPawnViewLocation(), Muzzle, BlasterHit::Channel, Params);
}

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming && IsValid(EquippedWeapon) && IsValid(Character) && !Character->IsEliminated();
	ServerSetAiming(bAiming);
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming && IsValid(EquippedWeapon) && IsValid(Character) && !Character->IsEliminated();
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!IsValid(Character) || !Character->IsLocallyControlled() || !Character->GetFollowCamera()) return;
	HitFeedback = FMath::Max(0.f, HitFeedback - DeltaTime);
	UCameraComponent* Camera = Character->GetFollowCamera();
	if (DefaultFOV <= 0.f) DefaultFOV = Camera->FieldOfView;
	const float TargetFOV = bAiming && IsValid(EquippedWeapon) ? 60.f : DefaultFOV;
	Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaTime, 15.f));
	if (Character->IsEliminated())
	{
		RecoverableRecoil = FVector2D::ZeroVector;
		ShotFeedback = 0.f;
		return;
	}
	ShotFeedback = FMath::FInterpTo(ShotFeedback, 0.f, DeltaTime, RecoverySpeed);
	// Recover only our own offset, preserving mouse input. Delay permits automatic climb.
	if (GetWorld()->GetTimeSeconds() - LastRecoilTime > 0.15 && !RecoverableRecoil.IsNearlyZero())
	{
		APlayerController* Controller = Cast<APlayerController>(Character->GetController());
		if (!Controller) return;
		const FVector2D Remaining = RecoverableRecoil * FMath::Exp(-RecoverySpeed * DeltaTime);
		const FVector2D Delta = RecoverableRecoil - Remaining;
		FRotator Rotation = Controller->GetControlRotation();
		const float MinPitch = Controller->PlayerCameraManager ? Controller->PlayerCameraManager->ViewPitchMin : -89.f;
		const float MaxPitch = Controller->PlayerCameraManager ? Controller->PlayerCameraManager->ViewPitchMax : 89.f;
		Rotation.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Rotation.Pitch) - Delta.X, MinPitch, MaxPitch);
		Rotation.Yaw -= Delta.Y;
		Controller->SetControlRotation(Rotation);
		RecoverableRecoil = Remaining;
	}
}

void UCombatComponent::ClientApplyRecoil_Implementation(AWeapon* FiredWeapon, float Pitch, float Yaw, float Recovery)
{
	if (!IsValid(Character) || !Character->IsLocallyControlled() || Character->IsEliminated()
		|| !IsValid(FiredWeapon) || FiredWeapon != EquippedWeapon) return;
	APlayerController* Controller = Cast<APlayerController>(Character->GetController());
	if (!Controller) return;
	FRotator Rotation = Controller->GetControlRotation();
	const float OldPitch = FRotator::NormalizeAxis(Rotation.Pitch);
	const float MinPitch = Controller->PlayerCameraManager ? Controller->PlayerCameraManager->ViewPitchMin : -89.f;
	const float MaxPitch = Controller->PlayerCameraManager ? Controller->PlayerCameraManager->ViewPitchMax : 89.f;
	Rotation.Pitch = FMath::Clamp(OldPitch + Pitch, MinPitch, MaxPitch);
	Rotation.Yaw += Yaw;
	Controller->SetControlRotation(Rotation);
	// Part of the kick remains as climb; the transient portion settles smoothly.
	RecoverableRecoil += FVector2D((Rotation.Pitch - OldPitch) * 0.65f, Yaw * 0.65f);
	RecoverySpeed = Recovery;
	LastRecoilTime = GetWorld()->GetTimeSeconds();
	ShotFeedback = FMath::Min(2.f, ShotFeedback + Pitch);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, TotalShotsFired);
	DOREPLIFETIME(UCombatComponent, CarriedAmmo);
	DOREPLIFETIME(UCombatComponent, bReloading);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
}

void UCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::Destroyed) DropWeapons();
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	bFireHeld = false;
	Super::EndPlay(EndPlayReason);
}

void UCombatComponent::Reload()
{
	if (IsValid(Character) && Character->IsLocallyControlled() && !Character->IsEliminated()) ServerReload();
}

bool UCombatComponent::AddCarriedAmmo(int32 Amount)
{
	if (!IsValid(Character) || !Character->HasAuthority() || Character->IsEliminated() || Amount <= 0 || CarriedAmmo >= 180) return false;
	CarriedAmmo += FMath::Min(Amount, 180 - CarriedAmmo);
	return true;
}

void UCombatComponent::ServerReload_Implementation()
{
	const AGameState* State = GetWorld()->GetGameState<AGameState>();
	if (!State || State->GetMatchState() != MatchState::InProgress) return;
	if (!IsValid(Character) || Character->IsEliminated() || !IsValid(EquippedWeapon)
		|| EquippedWeapon->GetOwner() != Character || bReloading || CarriedAmmo <= 0
		|| EquippedWeapon->GetAmmo() >= EquippedWeapon->GetMagazineCapacity()) return;
	bReloading = true;
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &UCombatComponent::FinishReload, 1.5f, false);
}

void UCombatComponent::FinishReload()
{
	const AGameState* State = GetWorld()->GetGameState<AGameState>();
	if (State && State->GetMatchState() == MatchState::InProgress
		&& IsValid(Character) && !Character->IsEliminated() && IsValid(EquippedWeapon)
		&& EquippedWeapon->GetOwner() == Character)
	{
		CarriedAmmo -= EquippedWeapon->AddAmmo(CarriedAmmo);
	}
	bReloading = false;
}

void UCombatComponent::SetFireButtonPressed(bool bPressed)
{
	if (!IsValid(Character) || !Character->IsLocallyControlled()) return;
	if (Character->IsEliminated()) bPressed = false;
	bFireHeld = bPressed;
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	if (bPressed)
	{
		FireOnce();
		GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireOnce,
			0.01f, true);
	}
}

void UCombatComponent::FireOnce()
{
	if (!bFireHeld || !IsValid(Character) || Character->IsEliminated() || !Character->IsLocallyControlled()
		|| !IsValid(EquippedWeapon) || bReloading
		|| !IsValid(Character->GetFollowCamera())) return;
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now < NextLocalFireTime) return;
	if (EquippedWeapon->GetAmmo() <= 0)
	{
		// Replication may not have delivered the reload state yet; avoid an RPC every poll.
		NextLocalFireTime = Now + 0.25;
		if (CarriedAmmo > 0) ServerReload();
		return;
	}
	FHitResult Hit;
	const AGameState* State = GetWorld()->GetGameState<AGameState>();
	if (!State || State->GetMatchState() != MatchState::InProgress) return;
	const APlayerState* PlayerState = Character->GetPlayerState();
	const double OneWayTime = PlayerState ? PlayerState->GetPingInMilliseconds() * 0.0005 : 0.0;
	NextLocalFireTime = Now + EquippedWeapon->GetFireInterval();
	ServerFire(TraceAim(Hit), EquippedWeapon, State->GetServerWorldTimeSeconds() - OneWayTime, NextLocalShotId++);
}

FVector UCombatComponent::TraceAim(FHitResult& Hit) const
{
	Hit = FHitResult();
	if (!IsValid(Character) || !Character->GetFollowCamera()) return FVector::ZeroVector;
	const UCameraComponent* Camera = Character->GetFollowCamera();
	const FVector Direction = Camera->GetForwardVector();
	// Start past the player's camera-to-body segment so near-camera scenery cannot
	// become a target behind the muzzle. The server separately checks muzzle clearance.
	const float BodyDistance = FMath::Max(0.f,
		FVector::DotProduct(Character->GetActorLocation() - Camera->GetComponentLocation(), Direction));
	const FVector Start = Camera->GetComponentLocation() + Direction * (BodyDistance + 30.f);
	const FVector End = Start + Direction * 80000.f;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BlasterAim), false, Character);
	if (IsValid(EquippedWeapon)) Params.AddIgnoredActor(EquippedWeapon);
	if (IsValid(SecondaryWeapon)) Params.AddIgnoredActor(SecondaryWeapon);
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, BlasterHit::Channel, Params);
	if (CVarShotDebug.GetValueOnGameThread())
	{
		DrawDebugLine(GetWorld(), Start, Hit.bBlockingHit ? Hit.ImpactPoint : End, FColor::Yellow, false, 0.f);
		if (Hit.bBlockingHit) DrawDebugString(GetWorld(), Hit.ImpactPoint, FString::Printf(TEXT("%s / %s / %s [WeaponTrace]"),
			*GetNameSafe(Hit.GetActor()), *GetNameSafe(Hit.GetComponent()), *Hit.BoneName.ToString()), nullptr, FColor::White, 0.f);
	}
	return Hit.bBlockingHit ? Hit.ImpactPoint : End;
}

void UCombatComponent::ServerFire_Implementation(FVector_NetQuantize HitTarget, AWeapon* RequestedWeapon, double ShotTime, int32 ShotId)
{
	const AGameState* State = GetWorld()->GetGameState<AGameState>();
	if (!State || State->GetMatchState() != MatchState::InProgress) return;
	if (ShotId <= LastServerShotId || ShotId <= 0) return;
	LastServerShotId = ShotId;
	const double Now = GetWorld()->GetTimeSeconds();
	const APlayerState* PlayerState = IsValid(Character) ? Character->GetPlayerState() : nullptr;
	const double AllowedAge = FMath::Min(1.0, 0.2 + (PlayerState ? PlayerState->GetPingInMilliseconds() * 0.001 : 0.0));
	if (!FMath::IsFinite(ShotTime) || ShotTime > Now + 0.05 || ShotTime < Now - AllowedAge
		|| RequestedWeapon != EquippedWeapon) return;
	if (!IsValid(Character) || Character->IsEliminated() || !Character->HasAuthority() || !Character->GetController()
		|| !IsValid(EquippedWeapon) || EquippedWeapon->GetOwner() != Character
		|| EquippedWeapon->GetAmmo() <= 0 || bReloading
		|| HitTarget.ContainsNaN() || GetWorld()->GetTimeSeconds() + 0.02 < NextServerFireTime) return;
	const USkeletalMeshComponent* Mesh = EquippedWeapon->GetWeaponMesh();
	if (!IsValid(Mesh) || !Mesh->DoesSocketExist(TEXT("Muzzle"))
		|| Mesh->GetAttachParent() != Character->GetMesh()) return;
	const FVector Muzzle = Mesh->GetSocketLocation(TEXT("Muzzle"));
	const FVector FromView = FVector(HitTarget) - Character->GetPawnViewLocation();
	const FVector ToTarget = FVector(HitTarget) - Muzzle;
	if (ToTarget.SizeSquared() < 1.f || FromView.SizeSquared() > FMath::Square(80500.f)
		|| FVector::DotProduct(FromView.GetSafeNormal(), Character->GetBaseAimRotation().Vector()) < 0.5f) return;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BlasterMuzzle), false, Character);
	Params.AddIgnoredActor(EquippedWeapon);
	FHitResult Obstruction;
	if (IsMuzzleBlocked(Obstruction)) return;
	TArray<FVector_NetQuantize> PelletEnds;
	if (EquippedWeapon->GetFireModel() == EFireModel::Projectile)
	{
		FActorSpawnParameters Spawn;
		Spawn.Owner = EquippedWeapon;
		Spawn.Instigator = Character;
		Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (!GetWorld()->SpawnActor<ABlasterProjectile>(Muzzle, ToTarget.Rotation(), Spawn)) return;
	}
	else if (EquippedWeapon->GetFireModel() == EFireModel::Hitscan)
	{
		AActor* Victim = nullptr;
		bool bHeadshot = false;
		if (ULagCompensationComponent::ConfirmHit(GetWorld(), Character, Muzzle,
			Muzzle + ToTarget.GetSafeNormal() * 80000.f, ShotTime, Victim, bHeadshot))
		{
			const float Applied = UGameplayStatics::ApplyDamage(Victim, BlasterHit::Damage(EquippedWeapon->GetBodyDamage(), EquippedWeapon->GetHeadMultiplier(), bHeadshot), Character->GetController(), EquippedWeapon, UDamageType::StaticClass());
			if (Applied > 0.f) ClientConfirmHit(bHeadshot);
		}
	}
	else
	{
		const bool bShotgun = EquippedWeapon->GetFireModel() == EFireModel::Shotgun;
		const int32 Pellets = bShotgun ? 8 : 1;
		TMap<AActor*, float> DamageByActor;
		bool bAnyHead = false;
		for (int32 Pellet = 0; Pellet < Pellets; ++Pellet)
		{
			const FVector Direction = bShotgun
				? FMath::VRandCone(ToTarget.GetSafeNormal(), FMath::DegreesToRadians(2.f)) : ToTarget.GetSafeNormal();
			FHitResult Hit;
			GetWorld()->LineTraceSingleByChannel(Hit, Muzzle, Muzzle + Direction * 80000.f, BlasterHit::Channel, Params);
			PelletEnds.Add(Hit.bBlockingHit ? Hit.ImpactPoint : Muzzle + Direction * 80000.f);
			if (auto* Victim = Cast<ABlasterCharacter>(Hit.GetActor()); Victim && !Victim->IsEliminated())
			{
				const bool bHead = BlasterHit::IsHead(Hit.BoneName);
				bAnyHead |= bHead;
				DamageByActor.FindOrAdd(Victim) += BlasterHit::Damage(EquippedWeapon->GetBodyDamage(), EquippedWeapon->GetHeadMultiplier(), bHead);
			}
		}
		for (const auto& Damage : DamageByActor)
		{
			if (IsValid(Damage.Key))
				UGameplayStatics::ApplyDamage(Damage.Key, Damage.Value, Character->GetController(), EquippedWeapon, UDamageType::StaticClass());
		}
		if (!DamageByActor.IsEmpty()) ClientConfirmHit(bAnyHead);
	}
	EquippedWeapon->SpendRound();
	// Allow one frame of arrival jitter without allowing the long-term cadence to drift faster.
	NextServerFireTime = FMath::Max(static_cast<double>(GetWorld()->GetTimeSeconds()), NextServerFireTime)
		+ EquippedWeapon->GetFireInterval();
	++TotalShotsFired;
	const float AimScale = bAiming ? 0.55f : 1.f;
	ClientApplyRecoil(EquippedWeapon, EquippedWeapon->GetRecoilPitch() * AimScale,
		FMath::FRandRange(-EquippedWeapon->GetRecoilYaw(), EquippedWeapon->GetRecoilYaw()) * AimScale,
		EquippedWeapon->GetRecoilRecovery());
	MulticastFire(Muzzle, HitTarget, PelletEnds);
	if (EquippedWeapon->GetAmmo() <= 0 && CarriedAmmo > 0) ServerReload_Implementation();
}

void UCombatComponent::MulticastFire_Implementation(FVector_NetQuantize MuzzleLocation, FVector_NetQuantize HitTarget, const TArray<FVector_NetQuantize>& PelletEnds)
{
	if (GetNetMode() == NM_DedicatedServer) return;
	if (!IsValid(Character) || !IsValid(EquippedWeapon)) return;
	UAnimInstance* Anim = Character->GetMesh()->GetAnimInstance();
	UAnimSequence* Sequence = bAiming && AimFireAnimation ? AimFireAnimation.Get() : HipFireAnimation.Get();
	if (Anim && Sequence)
	{
		Anim->PlaySlotAnimationAsDynamicMontage(Sequence, TEXT("DefaultSlot"), 0.025f, 0.05f);
	}
	// No ejection socket exists on this prototype weapon; use a receiver-relative position.
	const FVector Ejection = EquippedWeapon->GetActorLocation() + Character->GetActorRightVector() * 12.f;
	FActorSpawnParameters CosmeticSpawn;
	CosmeticSpawn.Owner = Character;
	CosmeticSpawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<ABlasterCasing>(Ejection, Character->GetActorRotation(), CosmeticSpawn);
	const FVector Direction = (FVector(HitTarget) - FVector(MuzzleLocation)).GetSafeNormal();
	const EFireModel Model = EquippedWeapon->GetFireModel();
	const FLinearColor Glow = Model == EFireModel::Hitscan ? FLinearColor(0.4f, 8.f, 14.f)
		: Model == EFireModel::Shotgun ? FLinearColor(14.f, 3.f, 0.3f) : FLinearColor(14.f, 9.f, 1.f);
	ABlasterShotEffect::Beam(GetWorld(), MuzzleLocation, FVector(MuzzleLocation) + Direction * 45.f, Glow, 16.f, 0.08f);
	// Projectile only gets a short muzzle streak; its moving tracer supplies the flight path.
	// Hitscan beams are cosmetic current-world traces, not a second damage calculation.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BlasterShotVisual), false, Character);
	Params.AddIgnoredActor(EquippedWeapon);
	const int32 Rays = Model == EFireModel::Shotgun ? FMath::Min(PelletEnds.Num(), 8) : 1;
	for (int32 Index = 0; Index < Rays; ++Index)
	{
		const FVector Ray = Model == EFireModel::Shotgun ? (FVector(PelletEnds[Index]) - FVector(MuzzleLocation)).GetSafeNormal() : Direction;
		FVector End = FVector(MuzzleLocation) + Ray * (Model == EFireModel::Projectile ? 400.f : 6000.f);
		if (Model == EFireModel::Shotgun && FVector::DistSquared(MuzzleLocation, PelletEnds[Index]) < FMath::Square(6000.f)) End = PelletEnds[Index];
		FHitResult VisualHit;
		if (GetWorld()->LineTraceSingleByChannel(VisualHit, MuzzleLocation, End, BlasterHit::Channel, Params)) End = VisualHit.ImpactPoint;
		ABlasterShotEffect::Beam(GetWorld(), MuzzleLocation, End, Glow, Model == EFireModel::Shotgun ? 2.5f : 4.f, 0.16f);
		if (Model != EFireModel::Projectile && VisualHit.bBlockingHit)
			ABlasterShotEffect::Beam(GetWorld(), End, End + VisualHit.ImpactNormal * 5.f, Glow, 9.f, 0.16f);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!IsValid(Character) || !Character->HasAuthority() || Character->IsEliminated()
		|| !IsValid(WeaponToEquip) || WeaponToEquip->GetOwner() || bReloading) return;
	if (!Character->GetMesh()->DoesSocketExist(TEXT("RightHandSocket"))) return;
	if (IsValid(EquippedWeapon))
	{
		if (!IsValid(SecondaryWeapon))
		{
			SecondaryWeapon = EquippedWeapon;
			SecondaryWeapon->SetActorHiddenInGame(true);
		}
		else EquippedWeapon->Drop();
	}
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetOwner(Character);
	AttachPrimary();
	Character->SetOverlappingWeapon(nullptr);
}

void UCombatComponent::AttachPrimary()
{
	if (!IsValid(Character) || !IsValid(EquippedWeapon)) return;
	EquippedWeapon->SetActorHiddenInGame(false);
	if (const USkeletalMeshSocket* Socket = Character->GetMesh()->GetSocketByName(TEXT("RightHandSocket")))
		Socket->AttachActor(EquippedWeapon, Character->GetMesh());
}

void UCombatComponent::SwapWeapons()
{
	if (IsValid(Character) && Character->IsLocallyControlled()) ServerSwapWeapons();
}

void UCombatComponent::ServerSwapWeapons_Implementation()
{
	if (!IsValid(Character) || Character->IsEliminated() || bReloading
		|| !IsValid(EquippedWeapon) || !IsValid(SecondaryWeapon)
		|| EquippedWeapon->GetOwner() != Character || SecondaryWeapon->GetOwner() != Character) return;
	Swap(EquippedWeapon, SecondaryWeapon);
	SecondaryWeapon->SetActorHiddenInGame(true);
	AttachPrimary();
}

void UCombatComponent::DropWeapons()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (IsValid(EquippedWeapon)) EquippedWeapon->Drop();
	if (IsValid(SecondaryWeapon))
	{
		SecondaryWeapon->Drop();
		SecondaryWeapon->AddActorWorldOffset(GetOwner()->GetActorRightVector() * 35.f, false, nullptr, ETeleportType::TeleportPhysics);
	}
	EquippedWeapon = nullptr;
	SecondaryWeapon = nullptr;
}

