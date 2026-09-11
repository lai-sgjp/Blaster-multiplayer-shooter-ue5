#include "StreetLobbyState.h"
#include "Net/UnrealNetwork.h"
void AStreetLobbyState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AStreetLobbyState, LaunchTime);
}
