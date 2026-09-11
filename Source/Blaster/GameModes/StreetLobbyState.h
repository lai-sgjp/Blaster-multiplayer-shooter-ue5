#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "StreetLobbyState.generated.h"
UCLASS()
class BLASTER_API AStreetLobbyState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
	UPROPERTY(Replicated, BlueprintReadOnly) double LaunchTime = 0;
};
