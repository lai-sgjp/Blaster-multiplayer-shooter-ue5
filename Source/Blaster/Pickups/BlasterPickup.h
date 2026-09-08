#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlasterPickup.generated.h"

UENUM()
enum class EBlasterPickupKind : uint8 { Health, Ammo, Speed };

UCLASS()
class BLASTER_API ABlasterPickup : public AActor
{
	GENERATED_BODY()
public:
	ABlasterPickup();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetKind(EBlasterPickupKind Value) { Kind = Value; }
	EBlasterPickupKind GetKind() const { return Kind; }
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UPROPERTY()
	TObjectPtr<class USphereComponent> Area;
	UPROPERTY()
	TObjectPtr<class UStaticMeshComponent> Visual;
	UPROPERTY()
	TObjectPtr<class UTextRenderComponent> Label;
	UPROPERTY(ReplicatedUsing = OnRep_Kind, VisibleInstanceOnly, Category = Pickup)
	EBlasterPickupKind Kind = EBlasterPickupKind::Health;
	UFUNCTION()
	void OnRep_Kind();
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Component, AActor* Actor, UPrimitiveComponent* Other,
		int32 BodyIndex, bool bSweep, const FHitResult& Hit);
	void Reactivate();
	bool bConsumed = false;
	FTimerHandle ReturnTimer;
};
