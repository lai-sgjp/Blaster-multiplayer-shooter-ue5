// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	void UpdateTurning(float DeltaSeconds);

	UPROPERTY(EditDefaultsOnly, Category = Turning)
	TObjectPtr<class UAnimSequence> TurnLeftAnimation;
	UPROPERTY(EditDefaultsOnly, Category = Turning)
	TObjectPtr<UAnimSequence> TurnRightAnimation;

	UPROPERTY(BlueprintReadOnly, Category = Turning, meta = (AllowPrivateAccess = "true"))
	float RootYawOffset = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = Turning, meta = (AllowPrivateAccess = "true"))
	float TurnRootYaw = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = Turning, meta = (AllowPrivateAccess = "true"))
	float TurnAnimationTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = Turning, meta = (AllowPrivateAccess = "true"))
	bool bTurningLeft = false;
	UPROPERTY(BlueprintReadOnly, Category = Turning, meta = (AllowPrivateAccess = "true"))
	bool bTurningRight = false;

	bool bWasTurningEligible = false;
	float PreviousTurnYaw = 0.f;
	float TurnRate = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class ABlasterCharacter* BlasterCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float YawOffset = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = IK, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = IK, meta = (AllowPrivateAccess = "true"))
	bool bLeftHandIKValid = false;

	FRotator PreviousRotation;
	
};
