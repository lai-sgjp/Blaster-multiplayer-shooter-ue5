// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	if (!InPawn) return;

	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole) {
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}

	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s\n%s"), *Role, *GetPlayerName());
	SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}

void UOverheadWidget::SetDisplayText(const FString TextToDisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

FString UOverheadWidget::GetPlayerName()
{
	if (APlayerState* PlayerState = GetOwningPlayerState()) {
		return PlayerState->GetPlayerName();
	}
	else return FString("Player Name Not Found");
}
