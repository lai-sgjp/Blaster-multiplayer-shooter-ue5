#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StreetWidget.generated.h"
class UTextBlock;
class UCanvasPanel;
class UVerticalBox;
class UButton;

UCLASS()
class BLASTER_API UStreetWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& CullRect, FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool bParentEnabled) const override;
	void Refresh();
private:
	UPROPERTY(Transient) TObjectPtr<class UFont> ScreenFont;
	UTextBlock* Text(UVerticalBox* Parent, const FString& Value, int32 Size, FLinearColor Color);
	UButton* Button(UVerticalBox* Parent, const FString& Value, FName Name);
	UVerticalBox* Panel(UCanvasPanel* Root, FVector2D Anchor, FVector2D Position, FVector2D Size, FVector2D Align);
	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Root;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> Menu;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> HealthPanel;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> AmmoPanel;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> TopPanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HealthText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> AmmoText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> AmmoName;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PhaseText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Details;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Notice;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Heading;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> EquipmentPanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PrimaryText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SecondaryText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SupplyText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> FeedbackText;
	UPROPERTY(Transient) TObjectPtr<UButton> HostButton;
	UPROPERTY(Transient) TObjectPtr<UButton> JoinButton;
	UPROPERTY(Transient) TObjectPtr<UButton> ReadyButton;
	UPROPERTY(Transient) TObjectPtr<UButton> MedicalButton;
	UPROPERTY(Transient) TObjectPtr<UButton> SpeedButton;
	UPROPERTY(Transient) TObjectPtr<UButton> ContinueButton;
	UPROPERTY(Transient) TObjectPtr<UButton> LeaveButton;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> SettingsPanel;
	float UpdateTime = 0;
	UFUNCTION() void Host();
	UFUNCTION() void Join();
	UFUNCTION() void Ready();
	UFUNCTION() void Medical();
	UFUNCTION() void Speed();
	UFUNCTION() void Continue();
	UFUNCTION() void Leave();
	UFUNCTION() void Settings();
	UFUNCTION() void SetSensitivity(float Value);
	UFUNCTION() void SetVolume(float Value);
};
