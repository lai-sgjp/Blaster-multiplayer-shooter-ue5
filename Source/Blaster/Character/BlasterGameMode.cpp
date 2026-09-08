// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "BlasterCharacter.h"
#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "BlasterPlayerState.h"
#include "BlasterGameState.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Pickups/BlasterPickup.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Engine/TargetPoint.h"

ABlasterGameMode::ABlasterGameMode()
{
	// set default pawn class to our character class
	DefaultPawnClass = ABlasterCharacter::StaticClass();
	PlayerControllerClass = ABlasterPlayerController::StaticClass();
	HUDClass = ABlasterHUD::StaticClass();
	PlayerStateClass = ABlasterPlayerState::StaticClass();
	GameStateClass = ABlasterGameState::StaticClass();
	bDelayedStart = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	const FName WeaponTags[] = {TEXT("Loot_Rifle"), TEXT("Loot_Carbine"), TEXT("Loot_Shotgun")};
	const FName BuffTags[] = {TEXT("Loot_Health"), TEXT("Loot_Ammo"), TEXT("Loot_Speed")};
	for (TActorIterator<ATargetPoint> Point(GetWorld()); Point; ++Point)
	{
		for (int32 Index = 0; Index < 3; ++Index)
		{
			if (Point->ActorHasTag(WeaponTags[Index]) && WeaponPickupClasses.IsValidIndex(Index) && WeaponPickupClasses[Index])
				GetWorld()->SpawnActor<AWeapon>(WeaponPickupClasses[Index], Point->GetActorLocation(), Point->GetActorRotation());
			if (Point->ActorHasTag(BuffTags[Index]))
			{
				const FTransform Transform = Point->GetActorTransform();
				if (auto* Pickup = GetWorld()->SpawnActorDeferred<ABlasterPickup>(ABlasterPickup::StaticClass(), Transform))
				{
					Pickup->SetKind(static_cast<EBlasterPickupKind>(Index));
					UGameplayStatics::FinishSpawningActor(Pickup, Transform);
				}
			}
		}
	}
	if (auto* State = GetGameState<ABlasterGameState>())
		State->SetPhaseDeadline(GetWorld()->GetTimeSeconds() + FMath::Max(1.f, WarmupTime));
}

AActor* ABlasterGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	const UClass* PawnClass = GetDefaultPawnClassForController(Player);
	const APawn* Template = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
	APlayerStart* Best = nullptr;
	double BestDistance = -1.0;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (!It->ActorHasTag(TEXT("DistributedSpawn"))) continue;
		FVector Location = It->GetActorLocation();
		if (Template && GetWorld()->EncroachingBlockingGeometry(Template, Location, It->GetActorRotation())) continue;
		double Nearest = TNumericLimits<double>::Max();
		for (TActorIterator<ABlasterCharacter> Other(GetWorld()); Other; ++Other)
		{
			if (Other->IsEliminated() || Other->GetController() == Player) continue;
			Nearest = FMath::Min(Nearest, FVector::DistSquared2D(Location, Other->GetActorLocation()));
		}
		if (!Best || Nearest > BestDistance)
		{
			Best = *It;
			BestDistance = Nearest;
		}
	}
	return Best ? Best : Super::ChoosePlayerStart_Implementation(Player);
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	auto* State = GetGameState<ABlasterGameState>();
	if (!State || GetWorld()->GetTimeSeconds() < State->GetPhaseDeadline()) return;
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		for (APlayerState* Player : State->PlayerArray)
		{
			if (auto* BlasterState = Cast<ABlasterPlayerState>(Player)) BlasterState->ResetRoundStats();
		}
		State->SetPhaseDeadline(GetWorld()->GetTimeSeconds() + FMath::Max(1.f, MatchTime));
		StartMatch();
	}
	else if (GetMatchState() == MatchState::InProgress)
	{
		State->SetPhaseDeadline(GetWorld()->GetTimeSeconds() + FMath::Max(1.f, CooldownTime));
		EndMatch();
	}
	else if (GetMatchState() == MatchState::WaitingPostMatch && !bRestartRequested)
	{
		bRestartRequested = true;
		RestartGame();
	}
}

void ABlasterGameMode::RecordElimination(AController* Victim, AController* Attacker)
{
	if (!HasAuthority()) return;
	if (auto* Match = GetGameState<ABlasterGameState>())
	{
		const FString VictimName = Victim && Victim->PlayerState ? Victim->PlayerState->GetPlayerName() : TEXT("Player");
		const FString AttackerName = Attacker && Attacker->PlayerState ? Attacker->PlayerState->GetPlayerName() : TEXT("World");
		Match->AnnounceElimination(Attacker == Victim
			? VictimName + TEXT(" was eliminated")
			: AttackerName + TEXT(" eliminated ") + VictimName);
	}
	if (Victim)
	{
		if (auto* State = Victim->GetPlayerState<ABlasterPlayerState>()) State->AddDefeat();
	}
	if (Attacker && Attacker != Victim)
	{
		if (APlayerState* State = Attacker->PlayerState)
		{
			State->SetScore(State->GetScore() + 1.f);
			State->ForceNetUpdate();
		}
	}
}

void ABlasterGameMode::RespawnPlayer(APawn* OldPawn, AController* Controller)
{
	if (!HasAuthority()) return;
	if (IsValid(Controller)) Controller->UnPossess();
	if (IsValid(OldPawn)) OldPawn->Destroy();
	if (IsValid(Controller)) RestartPlayer(Controller);
}
