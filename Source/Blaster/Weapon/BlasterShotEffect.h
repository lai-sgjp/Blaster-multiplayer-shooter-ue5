#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlasterShotEffect.generated.h"

// Local, collision-free presentation. Never participates in damage or replication.
UCLASS()
class BLASTER_API ABlasterShotEffect : public AActor
{
	GENERATED_BODY()
public:
	ABlasterShotEffect();
	static void Beam(UWorld* World, const FVector& Start, const FVector& End, const FLinearColor& Color,
		float Width = 1.5f, float Lifetime = 0.10f);
private:
	UPROPERTY()
	TObjectPtr<class UStaticMeshComponent> Visual;
};
