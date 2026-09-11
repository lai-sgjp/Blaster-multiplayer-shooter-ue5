// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SetFireButtonPressed(bool bPressed);
	FVector TraceAim(FHitResult& Hit) const;
	bool IsMuzzleBlocked(FHitResult& Hit) const;
	void StopForMenu() { SetFireButtonPressed(false); SetAiming(false); }
	AWeapon* GetSecondaryWeapon() const { return SecondaryWeapon; }
	UFUNCTION(Client, Reliable) void ClientConfirmHit(bool bHeadshot);
	float GetHitFeedback() const { return HitFeedback; }
	bool WasHeadshot() const { return bLastHeadshot; }
	void Reload();
	int32 GetCarriedAmmo() const { return CarriedAmmo; }
	bool IsReloading() const { return bReloading; }
	float GetShotFeedback() const { return ShotFeedback; }
	void SwapWeapons();
	void DropWeapons();
	bool AddCarriedAmmo(int32 Amount);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	void FireOnce();
	UFUNCTION(Server, Reliable)
	void ServerSwapWeapons();
	void AttachPrimary();
	UPROPERTY(Replicated)
	AWeapon* SecondaryWeapon = nullptr;
	UFUNCTION(Server, Reliable)
	void ServerReload();
	void FinishReload();
	FTimerHandle ReloadTimer;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = Ammo)
	int32 CarriedAmmo = 90;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = Ammo)
	bool bReloading = false;
	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize HitTarget, AWeapon* RequestedWeapon, double ShotTime, int32 ShotId);
	int32 NextLocalShotId = 1;
	int32 LastServerShotId = 0;
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFire(FVector_NetQuantize MuzzleLocation, FVector_NetQuantize HitTarget, const TArray<FVector_NetQuantize>& PelletEnds);

	UFUNCTION(Client, Reliable)
	void ClientApplyRecoil(AWeapon* FiredWeapon, float Pitch, float Yaw, float Recovery);
	UPROPERTY(VisibleInstanceOnly, Category = Handling)
	float ShotFeedback = 0.f;
	float HitFeedback = 0.f;
	bool bLastHeadshot = false;
	FVector2D RecoverableRecoil = FVector2D::ZeroVector;
	float RecoverySpeed = 5.f;
	double LastRecoilTime = -1.0;
	double NextLocalFireTime = 0.0;
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TObjectPtr<class UAnimSequence> HipFireAnimation;
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TObjectPtr<class UAnimSequence> AimFireAnimation;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = Combat)
	int32 TotalShotsFired = 0;
	FTimerHandle FireTimer;
	double NextServerFireTime = 0.0;
	bool bFireHeld = false;
	float DefaultFOV = 0.f;

	UPROPERTY(Replicated)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

public:	
	

};
