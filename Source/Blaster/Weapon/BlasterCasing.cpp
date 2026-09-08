#include "BlasterCasing.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ABlasterCasing::ABlasterCasing()
{
	InitialLifeSpan = 2.f;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Asset(TEXT(
		"/Game/Assets/FPS_Weapon_Bundle/Weapons/Meshes/Ammunition/SM_Shell_556x45_Empty.SM_Shell_556x45_Empty"));
	if (Asset.Succeeded()) Mesh->SetStaticMesh(Asset.Object);
	Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetGenerateOverlapEvents(false);
}

void ABlasterCasing::BeginPlay()
{
	Super::BeginPlay();
	Mesh->AddImpulse(GetActorRightVector() * 120.f + FVector::UpVector * 90.f, NAME_None, true);
	Mesh->AddAngularImpulseInDegrees(FVector(250.f, 130.f, 70.f), NAME_None, true);
}
