#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

UENUM(BlueprintType)
enum class ESupplyKind : uint8 { Medical, Speed };

UCLASS(ClassGroup=(Blaster), meta=(BlueprintSpawnableComponent))
class BLASTER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UInventoryComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
	bool Add(ESupplyKind Kind);
	void Clear();
	UFUNCTION(BlueprintPure) int32 Count(ESupplyKind Kind) const;
	UFUNCTION(Server, Reliable, BlueprintCallable) void ServerUse(ESupplyKind Kind);
	static bool CanAdd(int32 Count) { return Count >= 0 && Count < 3; }
private:
	UPROPERTY(Replicated) int32 Medical = 0;
	UPROPERTY(Replicated) int32 Speed = 0;
	double NextUseTime = 0;
};
