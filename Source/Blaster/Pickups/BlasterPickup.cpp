#include "BlasterPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

ABlasterPickup::ABlasterPickup()
{
	bReplicates = true;
	Area = CreateDefaultSubobject<USphereComponent>(TEXT("PickupArea"));
	SetRootComponent(Area);
	Area->InitSphereRadius(65.f);
	Area->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Area->SetCollisionResponseToAllChannels(ECR_Ignore);
	Area->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Area);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetRelativeScale3D(FVector(0.35f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Sphere.Succeeded()) Visual->SetStaticMesh(Sphere.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Glow(TEXT("/Game/Blueprints/Weapon/Effects/M_ShotGlow.M_ShotGlow"));
	if (Glow.Succeeded()) Visual->SetMaterial(0, Glow.Object);
	Visual->SetCastShadow(false);
	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Area);
	Label->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	Label->SetWorldSize(18.f);
	Label->SetHorizontalAlignment(EHTA_Center);
}

void ABlasterPickup::BeginPlay()
{
	Super::BeginPlay();
	OnRep_Kind();
	if (HasAuthority())
	{
		Area->OnComponentBeginOverlap.AddDynamic(this, &ABlasterPickup::OnOverlap);
		Area->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ABlasterPickup::OnRep_Kind()
{
	// Screen-facing, occlusion-aware names are drawn by the local HUD.
	Label->SetVisibility(false);
	Visual->SetRelativeScale3D(Kind == EBlasterPickupKind::Ammo ? FVector(0.35f, 0.18f, 0.23f) : FVector(0.24f));
	Visual->SetRelativeRotation(Kind == EBlasterPickupKind::Speed ? FRotator(0, 45, 45) : FRotator::ZeroRotator);
	if (auto* Material = Visual->CreateAndSetMaterialInstanceDynamic(0))
		Material->SetVectorParameterValue(TEXT("GlowColor"), Kind == EBlasterPickupKind::Health ? FLinearColor(0.15f, 2.f, 0.4f)
			: Kind == EBlasterPickupKind::Ammo ? FLinearColor(2.f, 1.2f, 0.1f) : FLinearColor(0.1f, 1.3f, 2.f));
	const TCHAR* Text = Kind == EBlasterPickupKind::Health ? TEXT("HEALTH +25")
		: Kind == EBlasterPickupKind::Ammo ? TEXT("AMMO +30") : TEXT("SPEED 8s");
	Label->SetText(FText::FromString(Text));
	Label->SetTextRenderColor(Kind == EBlasterPickupKind::Health ? FColor::Green
		: Kind == EBlasterPickupKind::Ammo ? FColor::Yellow : FColor::Cyan);
}

void ABlasterPickup::OnOverlap(UPrimitiveComponent* Component, AActor* Actor, UPrimitiveComponent* Other,
	int32 BodyIndex, bool bSweep, const FHitResult& Hit)
{
	if (!HasAuthority() || bConsumed) return;
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(Actor);
	const AGameState* State = GetWorld()->GetGameState<AGameState>();
	if (!Character || Character->IsEliminated() || !State || State->GetMatchState() != MatchState::InProgress) return;
	bool bApplied = false;
	if (Kind == EBlasterPickupKind::Health) bApplied = Character->Heal(25.f);
	else if (Kind == EBlasterPickupKind::Speed) bApplied = Character->ApplySpeedBuff();
	else if (auto* Combat = Character->FindComponentByClass<UCombatComponent>()) bApplied = Combat->AddCarriedAmmo(30);
	if (!bApplied) return;
	bConsumed = true;
	SetActorHiddenInGame(true);
	Area->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorldTimerManager().SetTimer(ReturnTimer, this, &ABlasterPickup::Reactivate, 30.f, false);
	ForceNetUpdate();
}

void ABlasterPickup::Reactivate()
{
	bConsumed = false;
	SetActorHiddenInGame(false);
	Area->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ForceNetUpdate();
}

void ABlasterPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ReturnTimer);
	Super::EndPlay(EndPlayReason);
}

void ABlasterPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPickup, Kind);
}
