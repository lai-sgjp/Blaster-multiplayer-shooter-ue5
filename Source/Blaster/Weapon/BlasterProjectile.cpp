#include "BlasterProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "BlasterShotEffect.h"
#include "Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/BlasterComponent/BlasterHitRules.h"
#include "Components/SkeletalMeshComponent.h"

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
	Collision->SetCollisionObjectType(BlasterHit::Channel);
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
		if (auto* Victim = Cast<ABlasterCharacter>(OtherActor); Victim && !Victim->IsEliminated())
		{
			const AWeapon* Weapon = Cast<AWeapon>(GetOwner());
			// Swept projectile contacts can report the receiving component without its
			// skeletal body. Refine against that mesh along the same flight segment.
			FHitResult BoneHit;
			FCollisionQueryParams Query(SCENE_QUERY_STAT(ProjectileBone), false, GetInstigator());
			const FVector Direction = Movement->Velocity.GetSafeNormal();
			const bool bRefined = Victim->GetMesh()->LineTraceComponent(BoneHit,
				Hit.TraceStart, Hit.TraceEnd + Direction * 10.f, Query);
			const FName Bone = bRefined ? BoneHit.BoneName : Hit.BoneName;
			const bool bHead = BlasterHit::IsHead(Bone);
			UE_LOG(LogTemp, VeryVerbose, TEXT("StreetProjectile bone=%s contact=%s refined=%d"), *Bone.ToString(), *Hit.BoneName.ToString(), bRefined);
			UGameplayStatics::ApplyDamage(Victim, BlasterHit::Damage(Weapon ? Weapon->GetBodyDamage() : 20.f,
				Weapon ? Weapon->GetHeadMultiplier() : 2.f, bHead), GetInstigatorController(), this, UDamageType::StaticClass());
			if (GetInstigator()) if (auto* Combat = GetInstigator()->FindComponentByClass<UCombatComponent>()) Combat->ClientConfirmHit(bHead);
		}
		Destroy();
	}
}
