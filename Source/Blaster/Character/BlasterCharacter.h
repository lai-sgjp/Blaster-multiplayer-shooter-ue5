// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BlasterCharacter.generated.h"

class UInputMappingContext;
class UInputComponent;
class UInputAction;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* BlasterMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DodgeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AimAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SwapAction;
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TSubclassOf<class AWeapon> DefaultWeaponClass;

	/**
	* Callback for Input
	*/
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);\
	void EKeyPressed();
	void CrouchButtonPressed();
	void Attack();
	void StopAttack();
	void ReloadButtonPressed();
	void SwapButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UInventoryComponent> Inventory;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class ULagCompensationComponent> LagCompensation;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = Stats)
	float Health = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Eliminated, VisibleInstanceOnly, Category = Stats)
	bool bEliminated = false;
	FTimerHandle RespawnTimer;
	FTimerHandle SpeedTimer;
	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed, VisibleInstanceOnly, Category = Buff)
	float MoveSpeed = 600.f;
	UFUNCTION()
	void OnRep_MoveSpeed();
	void ResetMoveSpeed();
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		AController* InstigatorController, AActor* DamageCauser);
	UFUNCTION()
	void OnRep_Eliminated();
	void Respawn();
	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidgetComponent;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	AWeapon* GetEquippedWeapon() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	bool IsAiming();
	bool IsEliminated() const { return bEliminated; }
	float GetHealth() const { return Health; }
	bool Heal(float Amount);
	bool ApplySpeedBuff();
};
