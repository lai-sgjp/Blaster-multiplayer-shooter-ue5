#include "InventoryComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameMode.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent() { SetIsReplicatedByDefault(true); }
void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UInventoryComponent, Medical, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryComponent, Speed, COND_OwnerOnly);
}
int32 UInventoryComponent::Count(ESupplyKind Kind) const
{
	return Kind == ESupplyKind::Medical ? Medical : Kind == ESupplyKind::Speed ? Speed : 0;
}
bool UInventoryComponent::Add(ESupplyKind Kind)
{
	const auto* Character = Cast<ABlasterCharacter>(GetOwner());
	if (!Character || !Character->HasAuthority() || Character->IsEliminated()
		|| (Kind != ESupplyKind::Medical && Kind != ESupplyKind::Speed) || !CanAdd(Count(Kind))) return false;
	++(Kind == ESupplyKind::Medical ? Medical : Speed);
	GetOwner()->ForceNetUpdate();
	return true;
}
void UInventoryComponent::Clear()
{
	if (!GetOwner()->HasAuthority()) return;
	Medical = Speed = 0;
	GetOwner()->ForceNetUpdate();
}
void UInventoryComponent::ServerUse_Implementation(ESupplyKind Kind)
{
	auto* Character = Cast<ABlasterCharacter>(GetOwner());
	const auto* State = GetWorld()->GetGameState<AGameState>();
	UE_LOG(LogTemp, Verbose, TEXT("InventoryUse authority=%d netmode=%d kind=%d count=%d match=%s alive=%d cooldown=%.3f"),
        GetOwner()->HasAuthority(), int32(GetNetMode()), int32(Kind), Count(Kind),
        State ? *State->GetMatchState().ToString() : TEXT("none"), Character && !Character->IsEliminated(),
        NextUseTime-GetWorld()->GetTimeSeconds());
	if (!Character || Character->IsEliminated() || !State || State->GetMatchState() != MatchState::InProgress
		|| Count(Kind) <= 0 || GetWorld()->GetTimeSeconds() < NextUseTime) return;
	const bool bUsed = Kind == ESupplyKind::Medical ? Character->Heal(25.f) : Character->ApplySpeedBuff();
	if (!bUsed) return;
	--(Kind == ESupplyKind::Medical ? Medical : Speed);
	NextUseTime = GetWorld()->GetTimeSeconds() + 0.25;
	Character->ForceNetUpdate();
}
