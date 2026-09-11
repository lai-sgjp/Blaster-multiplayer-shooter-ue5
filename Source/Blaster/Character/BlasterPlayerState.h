#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void AddDefeat();
	void ResetRoundStats();
	int32 GetDefeats() const { return Defeats; }
	bool IsReady() const { return bReady; }
	void SetReady(bool Value) { if (HasAuthority()) { bReady = Value; ForceNetUpdate(); } }
private:
	UPROPERTY(Replicated) bool bReady = false;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = Stats)
	int32 Defeats = 0;
};
