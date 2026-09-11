// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "StreetLobbyState.h"
#include "Blaster/Character/BlasterPlayerState.h"
#include "Blaster/Character/BlasterPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ALobbyGameMode::ALobbyGameMode()
{
	PlayerControllerClass = ABlasterPlayerController::StaticClass();
	PlayerStateClass = ABlasterPlayerState::StaticClass();
	GameStateClass = AStreetLobbyState::StaticClass();
	static ConstructorHelpers::FClassFinder<APawn> Pawn(TEXT("/Game/Blueprints/Character/BP_BlasterCharacter"));
	if (Pawn.Succeeded()) DefaultPawnClass = Pawn.Class;
	bUseSeamlessTravel = true;
	bDelayedStart = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	bHostRequested = false;

	if (auto* State = NewPlayer->GetPlayerState<ABlasterPlayerState>()) State->SetReady(false);
	if (auto* State = GetGameState<AStreetLobbyState>()) State->LaunchTime = 0;
}

void ALobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (GetNumPlayers() >= 8) ErrorMessage = TEXT("Room is full (8/8)");
}

void ALobbyGameMode::Logout(AController* Player)
{
	Super::Logout(Player);
	bHostRequested = false;
	if (auto* State = GetGameState<AStreetLobbyState>()) State->LaunchTime = 0;
}
void ALobbyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto* State = GetGameState<AStreetLobbyState>();
	if (!State || bTravelling) return;
	if (!bHostRequested || GetNumPlayers() < 3 || GetNumPlayers() > 8) { State->LaunchTime = 0; return; }
	if (State->LaunchTime <= 0) State->LaunchTime = GetWorld()->GetTimeSeconds() + 5;
	if (GetWorld()->GetTimeSeconds() >= State->LaunchTime)
	{
		bTravelling = true;
		GetWorld()->ServerTravel(TEXT("/Game/Street/Maps/StreetArena?listen"));
	}
}

void ALobbyGameMode::RequestStart(APlayerController* Requester)
{
	// Listen-server's local controller is the host. A remote client cannot spoof it.
	if (!Requester || !Requester->IsLocalController() || GetNumPlayers() < 3 || GetNumPlayers() > 8 || bTravelling) return;
	bHostRequested = !bHostRequested;
	if (auto* State = GetGameState<AStreetLobbyState>()) State->LaunchTime = 0;
}
