#include "LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "BlasterHitRules.h"

namespace
{
const FName Bones[] = {TEXT("Head"), TEXT("spine_02"), TEXT("pelvis"),
	TEXT("upperarm_l"), TEXT("upperarm_r"), TEXT("thigh_l"), TEXT("thigh_r")};
const FVector Extents[] = {FVector(16.f), FVector(30.f), FVector(25.f),
	FVector(20.f, 12.f, 12.f), FVector(20.f, 12.f, 12.f),
	FVector(22.f, 16.f, 16.f), FVector(22.f, 16.f, 16.f)};
}

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.f / 30.f;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(GetOwner()->HasAuthority());
	if (const auto* Character = Cast<ABlasterCharacter>(GetOwner())) AddTickPrerequisiteComponent(Character->GetMesh());
	History.Reserve(34);
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
	Super::TickComponent(DeltaTime, TickType, TickFunction);
	const auto* Character = Cast<ABlasterCharacter>(GetOwner());
	if (!Character || !Character->HasAuthority() || Character->IsEliminated()) return;
	const USkeletalMeshComponent* Mesh = Character->GetMesh();
	FFrame Frame;
	Frame.Time = GetWorld()->GetTimeSeconds();
	for (int32 Index = 0; Index < BoxCount; ++Index)
	{
		if (Mesh->GetBoneIndex(Bones[Index]) == INDEX_NONE) return;
		Frame.Boxes[Index] = Mesh->GetSocketTransform(Bones[Index], RTS_World);
	}
	History.Add(Frame);
	while (History.Num() > 34 || (History.Num() > 1 && Frame.Time - History[0].Time > 1.0))
		History.RemoveAt(0, 1, EAllowShrinking::No);
	NewestFrameTime = Frame.Time;
	HistoryFrames = History.Num();
}

bool ULagCompensationComponent::SampleFrame(double Time, FFrame& Out) const
{
	if (!FMath::IsFinite(Time) || History.IsEmpty() || Time < History[0].Time || Time > History.Last().Time + 0.1) return false;
	if (Time >= History.Last().Time) { Out = History.Last(); return true; }
	for (int32 Index = 1; Index < History.Num(); ++Index)
	{
		const FFrame& Newer = History[Index];
		if (Time > Newer.Time) continue;
		const FFrame& Older = History[Index - 1];
		const double Alpha = FMath::Clamp((Time - Older.Time) / FMath::Max(0.000001, Newer.Time - Older.Time), 0.0, 1.0);
		Out.Time = Time;
		for (int32 Box = 0; Box < BoxCount; ++Box)
		{
			Out.Boxes[Box].Blend(Older.Boxes[Box], Newer.Boxes[Box], Alpha);
		}
		return true;
	}
	return false;
}

bool ULagCompensationComponent::ConfirmHit(UWorld* World, AActor* Shooter, const FVector& Start, const FVector& End,
	double ShotTime, AActor*& Victim, bool& bHeadshot)
{
	Victim = nullptr;
	bHeadshot = false;
	if (!World || !Shooter || !Shooter->HasAuthority()) return false;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BlasterRewindWorld), false, Shooter);
	for (TActorIterator<ABlasterCharacter> It(World); It; ++It)
	{
		Params.AddIgnoredActor(*It);
		if (IsValid(It->GetEquippedWeapon())) Params.AddIgnoredActor(It->GetEquippedWeapon());
	}
	FHitResult WorldHit;
	World->LineTraceSingleByChannel(WorldHit, Start, End, BlasterHit::Channel, Params);
	float BestTime = WorldHit.bBlockingHit ? WorldHit.Time : 1.f;
	for (TActorIterator<ABlasterCharacter> It(World); It; ++It)
	{
		if (*It == Shooter || It->IsEliminated()) continue;
		const auto* Lag = It->FindComponentByClass<ULagCompensationComponent>();
		FFrame Frame;
		if (!Lag || !Lag->SampleFrame(ShotTime, Frame)) continue;
		for (int32 Box = 0; Box < BoxCount; ++Box)
		{
			const FVector LocalStart = Frame.Boxes[Box].InverseTransformPosition(Start);
			const FVector LocalEnd = Frame.Boxes[Box].InverseTransformPosition(End);
			FVector HitLocation, HitNormal;
			float HitTime = 0.f;
			if (FMath::LineExtentBoxIntersection(FBox(-Extents[Box], Extents[Box]), LocalStart, LocalEnd,
				FVector::ZeroVector, HitLocation, HitNormal, HitTime) && HitTime < BestTime)
			{
				BestTime = HitTime;
				Victim = *It;
				bHeadshot = Box == 0;
			}
		}
	}
	return Victim != nullptr;
}
