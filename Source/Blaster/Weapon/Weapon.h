// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireModel : uint8
{
	Projectile,
	Hitscan,
	Shotgun
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	UPROPERTY(EditDefaultsOnly, Category = Damage, meta = (ClampMin = "0"))
	float BodyDamage = 20.f;
	UPROPERTY(EditDefaultsOnly, Category = Damage, meta = (ClampMin = "0"))
	float PelletDamage = 4.f;
	UPROPERTY(EditDefaultsOnly, Category = Damage, meta = (ClampMin = "1"))
	float HeadMultiplier = 2.f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
	EFireModel FireModel = EFireModel::Projectile;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = Ammo)
	int32 Ammo = 30;
	UPROPERTY(EditDefaultsOnly, Category = Ammo, meta = (ClampMin = "1"))
	int32 MagazineCapacity = 30;
	UPROPERTY(EditDefaultsOnly, Category = Handling, meta = (ClampMin = "0.05"))
	float FireInterval = 0.10f;
	UPROPERTY(EditDefaultsOnly, Category = Handling, meta = (ClampMin = "0"))
	float RecoilPitch = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category = Handling, meta = (ClampMin = "0"))
	float RecoilYaw = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category = Handling, meta = (ClampMin = "0.1"))
	float RecoilRecovery = 5.f;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

public:	
	void SetWeaponState(EWeaponState State);
	void Drop();
	float GetBodyDamage() const { return FireModel == EFireModel::Shotgun ? PelletDamage : BodyDamage; }
	float GetHeadMultiplier() const { return HeadMultiplier; }
	int32 GetAmmo() const { return Ammo; }
	EFireModel GetFireModel() const { return FireModel; }
	int32 GetMagazineCapacity() const { return MagazineCapacity; }
	float GetFireInterval() const { return FMath::Max(0.05f, FireInterval); }
	float GetRecoilPitch() const { return FMath::Clamp(RecoilPitch, 0.f, 15.f); }
	float GetRecoilYaw() const { return FMath::Clamp(RecoilYaw, 0.f, 5.f); }
	float GetRecoilRecovery() const { return FMath::Max(0.1f, RecoilRecovery); }
	void SpendRound();
	int32 AddAmmo(int32 Amount);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
