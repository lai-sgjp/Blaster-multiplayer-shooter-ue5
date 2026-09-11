#include "BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "BlasterPlayerState.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/HUD/StreetWidget.h"
#include "Blaster/HUD/StreetSessionSubsystem.h"
#include "Blaster/GameModes/StreetLobbyState.h"
#include "Blaster/GameModes/LobbyGameMode.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/GameInstance.h"
#include "AudioDevice.h"

ABlasterPlayerController::ABlasterPlayerController()
{
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	InitializeScreen();
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

bool ABlasterPlayerController::IsStartScreen() const { return GetWorld()->GetMapName().Contains(TEXT("StreetStart")); }
bool ABlasterPlayerController::IsLobby() const { return GetWorld()->GetGameState<AStreetLobbyState>() != nullptr; }
void ABlasterPlayerController::InitializeScreen()
{
	if (!IsLocalController()) return;
	if (Screen) Screen->RemoveFromParent();
	UClass* WidgetClass = LoadClass<UStreetWidget>(nullptr, TEXT("/Game/Street/UI/W_StreetScreen.W_StreetScreen_C"));
	Screen = CreateWidget<UStreetWidget>(this, WidgetClass ? WidgetClass : UStreetWidget::StaticClass());
	Screen->AddToViewport(10);
	ScreenMap = GetWorld()->GetMapName();
	SetMenuOpen(IsStartScreen());
	if (IsStartScreen())
	{
		bAutoManageActiveCameraTarget = false;
		for (TActorIterator<ACameraActor> Camera(GetWorld()); Camera; ++Camera)
			if (Camera->ActorHasTag(TEXT("StartCamera"))) { SetViewTarget(*Camera); break; }
	}
	if (auto* Device = GetWorld()->GetAudioDeviceRaw())
		Device->SetTransientPrimaryVolume(GetGameInstance()->GetSubsystem<UStreetSessionSubsystem>()->Volume);
}
void ABlasterPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	InitializeScreen();
}
void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ThisClass::ToggleMenu);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ThisClass::ToggleMenu);
	InputComponent->BindKey(EKeys::F5, IE_Pressed, this, &ThisClass::ServerRequestStart);
}
void ABlasterPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (!IsLocalController()) return;
	FeedbackRemaining = FMath::Max(0.f, FeedbackRemaining - DeltaTime);
	if (!Screen || ScreenMap != GetWorld()->GetMapName()) InitializeScreen();
	if (IsStartScreen() && !Cast<ACameraActor>(GetViewTarget()))
		for (TActorIterator<ACameraActor> Camera(GetWorld()); Camera; ++Camera)
			if (Camera->ActorHasTag(TEXT("StartCamera"))) { SetViewTarget(*Camera); break; }
	if (const auto* LocalPawn = Cast<ABlasterCharacter>(GetPawn()); LocalPawn && LocalPawn->IsEliminated() && bMenuOpen)
		SetMenuOpen(false);
}
void ABlasterPlayerController::ToggleMenu()
{
	if (!IsStartScreen()) SetMenuOpen(!bMenuOpen);
}
void ABlasterPlayerController::SetMenuOpen(bool bOpen)
{
	if (!IsLocalController()) return;
	bMenuOpen = bOpen;
	if (auto* LocalPawn = Cast<ABlasterCharacter>(GetPawn()))
	{
		if (auto* Combat = LocalPawn->FindComponentByClass<UCombatComponent>()) Combat->StopForMenu();
		if (bOpen) { LocalPawn->StopJumping(); LocalPawn->GetCharacterMovement()->StopMovementImmediately(); }
		// Suppress the pawn input component (including fire/reload/swap), not the world.
		if (bOpen) LocalPawn->DisableInput(this); else LocalPawn->EnableInput(this);
	}
	SetIgnoreMoveInput(false); SetIgnoreLookInput(false);
	if (bOpen)
	{
		SetIgnoreMoveInput(true); SetIgnoreLookInput(true);
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		if (Screen) Mode.SetWidgetToFocus(Screen->TakeWidget());
		SetInputMode(Mode);
	}
	else { SetInputMode(FInputModeGameOnly()); }
	bShowMouseCursor = bOpen;
	if (Screen) Screen->Refresh();
}
void ABlasterPlayerController::ServerRequestStart_Implementation()
{
	if (!IsLobby()) return;
	if (auto* Lobby = GetWorld()->GetAuthGameMode<ALobbyGameMode>()) Lobby->RequestStart(this);
}
void ABlasterPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Screen) { Screen->RemoveFromParent(); Screen = nullptr; }
	Super::EndPlay(Reason);
}

void ABlasterPlayerController::ClientFeedback_Implementation(uint8 Kind)
{
    const TCHAR* Names[] = {TEXT("Body"), TEXT("Head"), TEXT("Pickup"), TEXT("Hurt")};
    if (Kind > 3 || !IsLocalController()) return;
    if (FeedbackSounds.IsEmpty())
        for (const TCHAR* Name : Names) FeedbackSounds.Add(LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("/Game/Street/Audio/%s.%s"), Name, Name)));
    if (auto* Sound = FeedbackSounds[Kind].Get()) UGameplayStatics::PlaySound2D(this, Sound, 0.55f);
    FeedbackMessage = Kind == 0 ? TEXT("命中") : Kind == 1 ? TEXT("爆头命中") : Kind == 2 ? TEXT("已拾取补给") : TEXT("受到攻击");
    FeedbackRemaining = Kind == 2 ? 1.2f : 0.45f;
}
