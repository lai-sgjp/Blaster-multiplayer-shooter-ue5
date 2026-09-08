#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlasterAnimationEditorLibrary.generated.h"

/** Project-specific, editor-only installation of the B01 turning pose branch. */
UCLASS()
class UNREALMCPBRIDGE_API UBlasterAnimationEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** Validates the known graph before writing; rolls back new nodes on failure. Does not save. */
	UFUNCTION(BlueprintCallable, Category = "Blaster Editor")
	static FString InstallTurningGraph();
	UFUNCTION(BlueprintCallable, Category = "Blaster Editor")
	static FString InstallFireSlot();
};
