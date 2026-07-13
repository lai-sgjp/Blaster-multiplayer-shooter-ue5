// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	UFUNCTION(BluePrintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

	UFUNCTION(BluePrintCallable)
	FString GetPlayerName();

protected:
	virtual void NativeDestruct() override;

public:
	void SetDisplayText(const FString TextToDisplay);
	
};
