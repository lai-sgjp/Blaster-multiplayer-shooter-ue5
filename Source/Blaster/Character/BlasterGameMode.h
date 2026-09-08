// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABlasterGameMode();
	void RecordElimination(AController* Victim, AController* Attacker);
	void RespawnPlayer(APawn* OldPawn, AController* Controller);
	virtual void Tick(float DeltaSeconds) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
protected:
	virtual void BeginPlay() override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override { return false; }
private:
	UPROPERTY(EditDefaultsOnly, Category = Pickups)
	TArray<TSubclassOf<class AWeapon>> WeaponPickupClasses;
	UPROPERTY(EditDefaultsOnly, Category = Match, meta = (ClampMin = "1"))
	float WarmupTime = 10.f;
	UPROPERTY(EditDefaultsOnly, Category = Match, meta = (ClampMin = "1"))
	float MatchTime = 120.f;
	UPROPERTY(EditDefaultsOnly, Category = Match, meta = (ClampMin = "1"))
	float CooldownTime = 10.f;
	bool bRestartRequested = false;
};
