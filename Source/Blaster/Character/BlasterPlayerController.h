// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABlasterPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void PostSeamlessTravel() override;
	UFUNCTION(BlueprintCallable) void ToggleMenu();
	UFUNCTION(BlueprintCallable) void SetMenuOpen(bool bOpen);
	UFUNCTION(BlueprintPure) bool IsMenuOpen() const { return bMenuOpen; }
	bool IsStartScreen() const;
	bool IsLobby() const;
	UFUNCTION(Server, Reliable) void ServerRequestStart();
	void InitializeScreen();
	UFUNCTION(Client, Unreliable) void ClientFeedback(uint8 Kind);
	FString FeedbackMessage;
	float FeedbackRemaining = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override; 
	// when the player controller possesses a pawn, this function will be called. 
	// We can use it to cast the pawn to our character class and store a reference to it.
private:
	UPROPERTY(Transient) TObjectPtr<class UStreetWidget> Screen;
	UPROPERTY(Transient) TArray<TObjectPtr<class USoundBase>> FeedbackSounds;
	bool bMenuOpen = false;
	FString ScreenMap;
};
