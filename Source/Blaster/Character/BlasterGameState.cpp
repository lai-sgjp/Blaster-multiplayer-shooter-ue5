#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState, PhaseDeadline);
	DOREPLIFETIME(ABlasterGameState, EliminationMessage);
	DOREPLIFETIME(ABlasterGameState, AnnouncementDeadline);
}

void ABlasterGameState::AnnounceElimination(const FString& Message)
{
	if (!HasAuthority()) return;
	EliminationMessage = Message.Left(160);
	AnnouncementDeadline = GetServerWorldTimeSeconds() + 5.0;
	ForceNetUpdate();
}

void ABlasterGameState::SetPhaseDeadline(double Value)
{
	if (HasAuthority())
	{
		PhaseDeadline = Value;
		ForceNetUpdate();
	}
}
