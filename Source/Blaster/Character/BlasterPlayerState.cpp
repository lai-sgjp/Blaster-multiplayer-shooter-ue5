#include "BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, bReady);
}

void ABlasterPlayerState::AddDefeat()
{
	if (HasAuthority())
	{
		++Defeats;
		ForceNetUpdate();
	}
}

void ABlasterPlayerState::ResetRoundStats()
{
	if (!HasAuthority()) return;
	SetScore(0.f);
	Defeats = 0;
	ForceNetUpdate();
}
