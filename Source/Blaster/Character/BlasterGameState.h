#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetPhaseDeadline(double Value);
	double GetPhaseDeadline() const { return PhaseDeadline; }
	void AnnounceElimination(const FString& Message);
	const FString& GetEliminationMessage() const { return EliminationMessage; }
	double GetAnnouncementDeadline() const { return AnnouncementDeadline; }
private:
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = Match)
	FString EliminationMessage;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = Match)
	double AnnouncementDeadline = 0.0;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = Match)
	double PhaseDeadline = 0.0;
};
