#include "BlasterShotEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

ABlasterShotEffect::ABlasterShotEffect()
{
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glow"));
	SetRootComponent(Visual);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetCastShadow(false);
	Visual->SetCanEverAffectNavigation(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Blueprints/Weapon/Effects/M_ShotGlow.M_ShotGlow"));
	if (Mesh.Succeeded()) Visual->SetStaticMesh(Mesh.Object);
	if (Material.Succeeded()) Visual->SetMaterial(0, Material.Object);
	InitialLifeSpan = 0.12f;
}

void ABlasterShotEffect::Beam(UWorld* World, const FVector& Start, const FVector& End,
	const FLinearColor& Color, float Width, float Lifetime)
{
	if (!World || World->GetNetMode() == NM_DedicatedServer) return;
	const FVector Delta = End - Start;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (auto* Effect = World->SpawnActor<ABlasterShotEffect>((Start + End) * 0.5f, Delta.Rotation(), Params))
	{
		Effect->SetActorScale3D(FVector(FMath::Max(Width, Delta.Size()) / 100.f, Width / 100.f, Width / 100.f));
		if (auto* Material = Effect->Visual->CreateAndSetMaterialInstanceDynamic(0))
			Material->SetVectorParameterValue(TEXT("GlowColor"), Color);
		Effect->SetLifeSpan(Lifetime);
	}
}
