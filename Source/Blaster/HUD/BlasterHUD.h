#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
private:
	UPROPERTY(Transient)
	TObjectPtr<class UFont> RuntimeFont;
	void DrawPickupLabels(class ABlasterCharacter* Character, float Scale);
	UPROPERTY(VisibleInstanceOnly, Category = Crosshair)
	float CrosshairSpread = 0.f;
	UPROPERTY(VisibleInstanceOnly, Category = Crosshair)
	bool bTargetCharacter = false;
};
