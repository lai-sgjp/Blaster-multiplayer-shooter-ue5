// Editor-only bridge for networking tests. Editor Python forces actor RPC callspace
// to Local, even for nested C++ calls; execute on a later engine tick instead.
#if WITH_EDITOR
#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "Containers/Ticker.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blaster/BlasterComponent/InventoryComponent.h"
namespace
{
FAutoConsoleCommandWithWorld QueueSpeedUse(
    TEXT("blaster.QAUseSpeed"), TEXT("PIE only: use the owning player's speed item on the next engine tick."),
    FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
    {
        if (!World || World->WorldType != EWorldType::PIE) return;
        TWeakObjectPtr<UWorld> WeakWorld(World);
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakWorld](float)
        {
            UWorld* Current = WeakWorld.Get();
            APlayerController* PC = Current ? Current->GetFirstPlayerController() : nullptr;
            APawn* Pawn = PC && PC->IsLocalController() ? PC->GetPawn() : nullptr;
            if (Pawn)
                if (auto* Inventory = Pawn->FindComponentByClass<UInventoryComponent>()) Inventory->ServerUse(ESupplyKind::Speed);
            return false;
        }), .1f);
    }));
}
#endif
