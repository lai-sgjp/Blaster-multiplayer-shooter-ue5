#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include <EnhancedInputSubsystems.h>
#include "InputMappingContext.h" 
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "BlasterGameMode.h"
#include "TimerManager.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Components/SkeletalMeshComponent.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->SetWalkableFloorAngle(46.f);
	GetCharacterMovement()->MaxStepHeight = 60.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent()); // original GetMesh()
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->SocketOffset = FVector(0.f, 70.f, 35.f);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm.q
	CameraBoom->bDoCollisionTest = true;               // 开启碰撞
	CameraBoom->ProbeSize = 12.f;                      // 探针大小
	CameraBoom->ProbeChannel = ECC_Camera;             // Camera 通道

	OverheadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidgetComponent"));
	OverheadWidgetComponent->SetupAttachment(GetRootComponent()); 

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, bEliminated);
	DOREPLIFETIME(ABlasterCharacter, MoveSpeed);
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	if (!HasAuthority() || bEliminated || !FMath::IsFinite(Damage) || Damage <= 0.f) return;
	const ABlasterGameMode* ActiveMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (!ActiveMode || ActiveMode->GetMatchState() != MatchState::InProgress) return;
	Health = FMath::Clamp(Health - Damage, 0.f, 100.f);
	if (Health <= 0.f)
	{
		bEliminated = true;
		OnRep_Eliminated();
		if (ABlasterGameMode* Mode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
			Mode->RecordElimination(GetController(), InstigatorController);
		if (Combat) Combat->DropWeapons();
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &ABlasterCharacter::Respawn, 3.f, false);
	}
	ForceNetUpdate();
}

void ABlasterCharacter::OnRep_Eliminated()
{
	if (!bEliminated) return;
	GetWorldTimerManager().ClearTimer(SpeedTimer);
	if (Combat) Combat->SetFireButtonPressed(false);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	SetActorEnableCollision(false);
}

void ABlasterCharacter::Respawn()
{
	if (ABlasterGameMode* Mode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
		Mode->RespawnPlayer(this, GetController());
}

void ABlasterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RespawnTimer);
	GetWorldTimerManager().ClearTimer(SpeedTimer);
	Super::EndPlay(EndPlayReason);
}

bool ABlasterCharacter::Heal(float Amount)
{
	if (!HasAuthority() || bEliminated || !FMath::IsFinite(Amount) || Amount <= 0.f || Health >= 100.f) return false;
	Health = FMath::Min(100.f, Health + Amount);
	ForceNetUpdate();
	return true;
}

bool ABlasterCharacter::ApplySpeedBuff()
{
	if (!HasAuthority() || bEliminated) return false;
	MoveSpeed = 900.f;
	OnRep_MoveSpeed();
	GetWorldTimerManager().SetTimer(SpeedTimer, this, &ABlasterCharacter::ResetMoveSpeed, 8.f, false);
	ForceNetUpdate();
	return true;
}

void ABlasterCharacter::ResetMoveSpeed()
{
	MoveSpeed = 600.f;
	OnRep_MoveSpeed();
	ForceNetUpdate();
}

void ABlasterCharacter::OnRep_MoveSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	if (HasAuthority()) GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	if (HasAuthority()) OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	if (HasAuthority() && Combat && DefaultWeaponClass)
	{
		FActorSpawnParameters Spawn;
		Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, GetActorLocation(), GetActorRotation(), Spawn);
		if (DefaultWeapon) Combat->EquipWeapon(DefaultWeapon);
	}

	UE_LOG(LogTemp, Warning, TEXT("BlasterMappingContext is: %s"),BlasterMappingContext ? *BlasterMappingContext->GetName() : TEXT("NULL"));
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(BlasterMappingContext, 0);
		}
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Both the owning player and server use the same rotation policy. Proxies
	// consume the replicated actor rotation; animation never rotates the actor.
	const bool bEquipped = IsValid(GetEquippedWeapon());
	bUseControllerRotationYaw = bEquipped;
	GetCharacterMovement()->bOrientRotationToMovement = !bEquipped;
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController())) {
		if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Jump);

			EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ABlasterCharacter::EKeyPressed);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlasterCharacter::CrouchButtonPressed);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABlasterCharacter::AimButtonPressed);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlasterCharacter::AimButtonReleased);
			if (AttackAction)
			{
				EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABlasterCharacter::Attack);
				EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ABlasterCharacter::StopAttack);
				EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Canceled, this, &ABlasterCharacter::StopAttack);
			}
			if (ReloadAction)
				EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ABlasterCharacter::ReloadButtonPressed);
			if (SwapAction)
				EnhancedInputComponent->BindAction(SwapAction, ETriggerEvent::Started, this, &ABlasterCharacter::SwapButtonPressed);
		}
	}
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller) {
		const FRotator YawRotation(0.f, GetController()->GetControlRotation().Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y); // W & S are in Y axis
		AddMovementInput(RightDirection, MovementVector.X); // A & D are in X axis
	}
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController()) {
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void ABlasterCharacter::EKeyPressed()
{
	if (Combat) ServerEquipButtonPressed();
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::Attack()
{
	if (IsValid(Combat)) Combat->SetFireButtonPressed(true);
}

void ABlasterCharacter::StopAttack()
{
	if (IsValid(Combat)) Combat->SetFireButtonPressed(false);
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (IsValid(Combat)) Combat->Reload();
}

void ABlasterCharacter::SwapButtonPressed()
{
	if (IsValid(Combat)) Combat->SwapWeapons();
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		// Re-query actual overlaps: another nearby pickup can remain after one is
		// equipped or leaves, without generating a new begin-overlap event.
		TArray<AActor*> Candidates;
		GetOverlappingActors(Candidates, AWeapon::StaticClass());
		AWeapon* Nearest = nullptr;
		double BestDistance = TNumericLimits<double>::Max();
		for (AActor* Candidate : Candidates)
		{
			AWeapon* Weapon = Cast<AWeapon>(Candidate);
			if (!IsValid(Weapon) || Weapon->GetOwner()) continue;
			const double Distance = FVector::DistSquared(GetActorLocation(), Weapon->GetActorLocation());
			if (Distance < BestDistance) { Nearest = Weapon; BestDistance = Distance; }
		}
		Combat->EquipWeapon(Nearest);
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
	return IsValid(Combat) ? Combat->EquippedWeapon : nullptr;
}
