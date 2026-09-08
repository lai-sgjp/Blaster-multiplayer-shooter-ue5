#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlasterCasing.generated.h"

/** Local cosmetic only: never participates in gameplay collision or replication. */
UCLASS()
class BLASTER_API ABlasterCasing : public AActor
{
	GENERATED_BODY()
public:
	ABlasterCasing();
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	TObjectPtr<class UStaticMeshComponent> Mesh;
};
