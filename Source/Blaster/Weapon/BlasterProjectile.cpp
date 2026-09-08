#include "BlasterProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "BlasterShotEffect.h"

ABlasterProjectile::ABlasterProjectile()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.025f;
	SetReplicateMovement(true);
	InitialLifeSpan = 3.f;
	SetNetUpdateFrequency(30.f);
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(3.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Collision);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetRelativeScale3D(FVector(0.06f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere.Succeeded()) Visual->SetStaticMesh(Sphere.Object);
	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	Movement->SetUpdatedComponent(Collision);
	Movement->InitialSpeed = 10000.f;
	Movement->MaxSpeed = 10000.f;
	Movement->ProjectileGravityScale = 0.f;
	Movement->bRotationFollowsVelocity = true;
}

void ABlasterProjectile::BeginPlay()
{
	Super::BeginPlay();
	PreviousVisualLocation = GetActorLocation();
	Collision->IgnoreActorWhenMoving(GetOwner(), true);
	Collision->IgnoreActorWhenMoving(GetInstigator(), true);
	if (HasAuthority())
	{
		Collision->OnComponentHit.AddDynamic(this, &ABlasterProjectile::OnHit);
	}
	else
	{
		// Clients advance the visible projectile between replicated corrections.
		// Only the server decides collision and destruction.
		Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABlasterProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ABlasterShotEffect::Beam(GetWorld(), PreviousVisualLocation, GetActorLocation(), FLinearColor(14.f, 9.f, 1.f), 4.f, 0.12f);
	PreviousVisualLocation = GetActorLocation();
}

void ABlasterProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (HasAuthority() && OtherActor != GetInstigator() && OtherActor != GetOwner())
	{
		if (IsValid(OtherActor))
			UGameplayStatics::ApplyDamage(OtherActor, 20.f, GetInstigatorController(), this, UDamageType::StaticClass());
		Destroy();
	}
}
