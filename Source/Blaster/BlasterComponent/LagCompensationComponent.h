#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/StaticArray.h"
#include "LagCompensationComponent.generated.h"

UCLASS(ClassGroup = Combat)
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;
	static bool ConfirmHit(UWorld* World, AActor* Shooter, const FVector& Start, const FVector& End,
		double ShotTime, AActor*& Victim, bool& bHeadshot);
protected:
	virtual void BeginPlay() override;
private:
	static constexpr int32 BoxCount = 7;
	struct FFrame
	{
		double Time = 0.0;
		TStaticArray<FTransform, BoxCount> Boxes;
	};
	TArray<FFrame> History;
	bool SampleFrame(double Time, FFrame& Out) const;
	UPROPERTY(VisibleInstanceOnly, Category = Rewind)
	double NewestFrameTime = 0.0;
	UPROPERTY(VisibleInstanceOnly, Category = Rewind)
	int32 HistoryFrames = 0;
};
