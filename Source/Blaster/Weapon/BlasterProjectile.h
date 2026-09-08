#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlasterProjectile.generated.h"

UCLASS()
class BLASTER_API ABlasterProjectile : public AActor
{
	GENERATED_BODY()
public:
	ABlasterProjectile();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

private:
	FVector PreviousVisualLocation = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> Collision;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UStaticMeshComponent> Visual;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UProjectileMovementComponent> Movement;
};
