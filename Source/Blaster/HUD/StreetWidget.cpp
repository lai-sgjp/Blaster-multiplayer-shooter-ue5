#include "StreetWidget.h"
#include "Blaster/BlasterComponent/BlasterHitRules.h"
#include "Rendering/DrawElements.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Blaster/Character/BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Character/BlasterPlayerState.h"
#include "Blaster/Character/BlasterGameState.h"
#include "Blaster/BlasterComponent/InventoryComponent.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/GameModes/StreetLobbyState.h"
#include "StreetSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"
#include "GameFramework/GameMode.h"
#include "Engine/Font.h"

namespace
{
const FLinearColor Paper(0.95f, 0.92f, 0.85f);
const FLinearColor Muted(0.55f, 0.57f, 0.58f);
const FLinearColor Accent(1.f, 0.49f, 0.12f);
FString WeaponName(const AWeapon* Weapon)
{
	if (!IsValid(Weapon)) return TEXT("空武器位");
	return Weapon->GetFireModel() == EFireModel::Shotgun ? TEXT("霰弹枪")
		: Weapon->GetFireModel() == EFireModel::Hitscan ? TEXT("卡宾枪") : TEXT("突击步枪");
}
}
UTextBlock* UStreetWidget::Text(UVerticalBox* Parent, const FString& Value, int32 Size, FLinearColor Color)
{
	auto* Label = WidgetTree->ConstructWidget<UTextBlock>();
	Label->SetText(FText::FromString(Value));
	FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Bold", Size);
	if (ScreenFont) Font = FSlateFontInfo(ScreenFont, Size);
	Label->SetFont(Font);
	Label->SetColorAndOpacity(Color);
	Label->SetAutoWrapText(true);
	Parent->AddChildToVerticalBox(Label)->SetPadding(FMargin(0, 4, 0, 6));
	return Label;
}
UButton* UStreetWidget::Button(UVerticalBox* Parent, const FString& Value, FName Name)
{
	auto* Result = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	FButtonStyle Style;
	FSlateBrush Normal; Normal.DrawAs = ESlateBrushDrawType::RoundedBox; Normal.OutlineSettings.CornerRadii = FVector4(9,9,9,9); Normal.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius; Normal.TintColor = FSlateColor(FLinearColor(0.065f,0.065f,0.065f));
	FSlateBrush Hover = Normal; Hover.TintColor = FSlateColor(FLinearColor(0.24f,0.16f,0.10f));
	FSlateBrush Pressed = Normal; Pressed.TintColor = FSlateColor(FLinearColor(0.40f,0.21f,0.08f));
	Style.SetNormal(Normal).SetHovered(Hover).SetPressed(Pressed);
	Style.SetNormalPadding(FMargin(18,9)).SetPressedPadding(FMargin(18,10,18,8));
	Result->SetStyle(Style);
	auto* Label = WidgetTree->ConstructWidget<UTextBlock>();
	Label->SetText(FText::FromString(Value));
	FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Bold", 18);
	if (ScreenFont) Font = FSlateFontInfo(ScreenFont, 18);
	Label->SetFont(Font); Label->SetColorAndOpacity(Paper);
	Result->AddChild(Label);
	Parent->AddChildToVerticalBox(Result)->SetPadding(FMargin(0,3));
	return Result;
}
UVerticalBox* UStreetWidget::Panel(UCanvasPanel* Canvas, FVector2D Anchor, FVector2D Position, FVector2D Size, FVector2D Align)
{
	auto* Border = WidgetTree->ConstructWidget<UBorder>();
	Border->SetBrushColor(FLinearColor(0.018f,0.018f,0.018f,0.94f));
	Border->SetPadding(FMargin(22,14));
	auto* CanvasSlot = Canvas->AddChildToCanvas(Border);
	CanvasSlot->SetAnchors(FAnchors(Anchor.X,Anchor.Y)); CanvasSlot->SetAlignment(Align);
	CanvasSlot->SetPosition(Position); CanvasSlot->SetSize(Size);
	auto* Box = WidgetTree->ConstructWidget<UVerticalBox>(); Border->AddChild(Box);
	return Box;
}
void UStreetWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
	// Reticle follows this local player viewport every frame, including small client PIE windows.
	ForceVolatile(true);
	ScreenFont = NewObject<UFont>(this);
	ScreenFont->FontCacheType = EFontCacheType::Runtime;
	FTypefaceEntry Entry(FName(TEXT("Bold")));
	Entry.Font = FFontData(LoadObject<UObject>(nullptr, TEXT("/Game/Street/UI/StreetFont.StreetFont")));
	ScreenFont->CompositeFont.DefaultTypeface.Fonts.Add(Entry);
	Root = WidgetTree->ConstructWidget<UCanvasPanel>(); WidgetTree->RootWidget = Root;
	HealthPanel = Panel(Root,{0,1},{30,-30},{260,110},{0,1});
	Text(HealthPanel,TEXT("生命"),14,Muted); HealthText = Text(HealthPanel,TEXT("100 / 100"),28,Paper);
	AmmoPanel = Panel(Root,{1,1},{-30,-30},{280,110},{1,1});
	AmmoName = Text(AmmoPanel,TEXT("装备"),14,Muted); AmmoText = Text(AmmoPanel,TEXT(""),23,Paper);
	TopPanel = Panel(Root,{0.5,0},{0,24},{560,96},{0.5,0});
	PhaseText = Text(TopPanel,TEXT("街区交火"),20,Paper);
	FeedbackText = Text(TopPanel,TEXT(""),14,Accent);
	Menu = Panel(Root,{0.5,0.5},{0,0},{920,780},{0.5,0.5});
	Heading = Text(Menu,TEXT("街区交火"),40,Paper);
	Text(Menu,TEXT("车站街区  /  自由混战"),15,Accent);
	Details = Text(Menu,TEXT(""),18,Paper);
	Notice = Text(Menu,TEXT(""),15,Muted);
	HostButton = Button(Menu,TEXT("创建房间"),TEXT("Host")); HostButton->OnClicked.AddDynamic(this,&ThisClass::Host);
	JoinButton = Button(Menu,TEXT("加入房间"),TEXT("Join")); JoinButton->OnClicked.AddDynamic(this,&ThisClass::Join);
	ReadyButton = Button(Menu,TEXT("房主开始 / 取消出发   F5"),TEXT("Ready")); ReadyButton->OnClicked.AddDynamic(this,&ThisClass::Ready);
	EquipmentPanel = WidgetTree->ConstructWidget<UVerticalBox>(); Menu->AddChild(EquipmentPanel);
    auto* Row = WidgetTree->ConstructWidget<UHorizontalBox>(); EquipmentPanel->AddChild(Row);
    auto Card = [&](const TCHAR* Title)
    {
        auto* Frame = WidgetTree->ConstructWidget<UBorder>();
        Frame->SetBrushColor(FLinearColor(0.075f,0.075f,0.075f)); Frame->SetPadding(FMargin(18,12));
        auto* CardSlot = Row->AddChildToHorizontalBox(Frame); CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); CardSlot->SetPadding(FMargin(0,3,10,3));
        auto* Stack = WidgetTree->ConstructWidget<UVerticalBox>(); Frame->AddChild(Stack);
        Text(Stack,Title,14,Accent); return Stack;
    };
    auto* PrimaryCard=Card(TEXT("01  /  当前武器")); PrimaryText=Text(PrimaryCard,TEXT(""),23,Paper);
    auto* SecondaryCard=Card(TEXT("02  /  备用武器")); SecondaryText=Text(SecondaryCard,TEXT(""),23,Paper);
    SupplyText=Text(EquipmentPanel,TEXT(""),17,Paper);
    MedicalButton = Button(EquipmentPanel,TEXT("使用医疗包   +25 生命"),TEXT("Medical")); MedicalButton->OnClicked.AddDynamic(this,&ThisClass::Medical);
	SpeedButton = Button(EquipmentPanel,TEXT("使用加速道具   8 秒"),TEXT("Speed")); SpeedButton->OnClicked.AddDynamic(this,&ThisClass::Speed);
	ContinueButton = Button(Menu,TEXT("继续游戏   Tab / Esc"),TEXT("Continue")); ContinueButton->OnClicked.AddDynamic(this,&ThisClass::Continue);
	auto* SettingsButton = Button(Menu,TEXT("设置"),TEXT("Settings")); SettingsButton->OnClicked.AddDynamic(this,&ThisClass::Settings);
	SettingsPanel = WidgetTree->ConstructWidget<UVerticalBox>(); Menu->AddChild(SettingsPanel);
	auto* Session = GetGameInstance()->GetSubsystem<UStreetSessionSubsystem>();
	Text(SettingsPanel,TEXT("鼠标灵敏度"),14,Muted);
	auto* Sens = WidgetTree->ConstructWidget<USlider>(); Sens->SetMinValue(0.2f); Sens->SetMaxValue(3.f); Sens->SetValue(Session->Sensitivity);
	Sens->OnValueChanged.AddDynamic(this,&ThisClass::SetSensitivity); SettingsPanel->AddChild(Sens);
	Text(SettingsPanel,TEXT("主音量"),14,Muted);
	auto* Volume = WidgetTree->ConstructWidget<USlider>(); Volume->SetValue(Session->Volume);
	Volume->OnValueChanged.AddDynamic(this,&ThisClass::SetVolume); SettingsPanel->AddChild(Volume);
	SettingsPanel->SetVisibility(ESlateVisibility::Collapsed);
	LeaveButton = Button(Menu,TEXT("返回开始界面"),TEXT("Leave")); LeaveButton->OnClicked.AddDynamic(this,&ThisClass::Leave);
	Refresh();
}
void UStreetWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry,DeltaTime);
	UpdateTime += DeltaTime;
	if (UpdateTime >= 0.1f) { UpdateTime = 0; Refresh(); }
}
FReply UStreetWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (Event.GetKey() == EKeys::Escape || Event.GetKey() == EKeys::Tab)
	{
		if (auto* PC = GetOwningPlayer<ABlasterPlayerController>()) PC->ToggleMenu();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(Geometry,Event);
}
void UStreetWidget::Refresh()
{
	auto* PC = GetOwningPlayer<ABlasterPlayerController>();
	if (!PC || !Menu) return;
	auto* Session = GetGameInstance()->GetSubsystem<UStreetSessionSubsystem>();
	const bool bStart = PC->IsStartScreen(), bLobby = PC->IsLobby();
	if (auto* FrameSlot = Cast<UCanvasPanelSlot>(Menu->GetParent()->Slot))
	{
		FrameSlot->SetAnchors(bStart ? FAnchors(0,0.5f) : FAnchors(0.5f,0.5f));
		FrameSlot->SetAlignment(bStart ? FVector2D(0,0.5f) : FVector2D(0.5f,0.5f));
		FrameSlot->SetPosition(bStart ? FVector2D(40,0) : FVector2D::ZeroVector);
		FrameSlot->SetSize(bStart ? FVector2D(430,670) : FVector2D(920,780));
	}
	Menu->GetParent()->SetVisibility(PC->IsMenuOpen() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	HealthPanel->GetParent()->SetVisibility(bStart || bLobby ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	AmmoPanel->GetParent()->SetVisibility(bStart || bLobby ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	TopPanel->GetParent()->SetVisibility(bStart ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	auto Show = [](UWidget* Widget, bool Value) { Widget->SetVisibility(Value ? ESlateVisibility::Visible : ESlateVisibility::Collapsed); };
	Show(HostButton,bStart); Show(JoinButton,bStart); Show(ReadyButton,bLobby); Show(ContinueButton,!bStart);
	Show(EquipmentPanel,!bStart && !bLobby && SettingsPanel->GetVisibility()==ESlateVisibility::Collapsed);
	Show(Details,SettingsPanel->GetVisibility()==ESlateVisibility::Collapsed); Show(MedicalButton,!bStart && !bLobby); Show(SpeedButton,!bStart && !bLobby);
	HostButton->SetIsEnabled(!Session->bBusy); JoinButton->SetIsEnabled(!Session->bBusy);
	Heading->SetText(FText::FromString(bStart ? TEXT("街区交火") : bLobby ? TEXT("出发大厅") : TEXT("随身装备")));
	Cast<UTextBlock>(LeaveButton->GetContent())->SetText(FText::FromString(bStart ? TEXT("退出游戏") : TEXT("离开房间")));
	Notice->SetText(FText::FromString(Session->Notice.IsEmpty() && !bStart && !bLobby ? TEXT("对局仍在继续，打开背包时你仍会受到伤害。") : Session->Notice));
	FeedbackText->SetText(FText::FromString(PC->FeedbackRemaining > 0 ? PC->FeedbackMessage : TEXT("Tab / Esc  随身装备")));
	const auto* Character = Cast<ABlasterCharacter>(PC->GetPawn());
	const auto* Stats = PC->GetPlayerState<ABlasterPlayerState>();
	if (bStart) Details->SetText(FText::FromString(TEXT("穿过车站，加入交火。\nSteam 联机 · 3–8 人 · 房主开始")));
	else if (const auto* Lobby = GetWorld()->GetGameState<AStreetLobbyState>())
	{
		FString Roster;
		for (const APlayerState* Player : Lobby->PlayerArray)
		{
			Roster += FString::Printf(TEXT("%s\n"), *Player->GetPlayerName().Left(24));
		}
		Details->SetText(FText::FromString(Roster));
		PhaseText->SetText(FText::FromString(Lobby->LaunchTime > 0
			? FString::Printf(TEXT("%d 秒后出发"),FMath::Max(0,FMath::CeilToInt(Lobby->LaunchTime-Lobby->GetServerWorldTimeSeconds())))
			: FString::Printf(TEXT("出发大厅  %d / 8   ·   至少3人，由房主开始"),Lobby->PlayerArray.Num())));
		ReadyButton->SetIsEnabled(PC->HasAuthority() && Lobby->PlayerArray.Num()>=3);
	}
	else if (Character)
	{
		const auto* Combat = Character->FindComponentByClass<UCombatComponent>();
		const auto* Inventory = Character->FindComponentByClass<UInventoryComponent>();
		const auto* Weapon = Character->GetEquippedWeapon();
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / 100"),Character->GetHealth())));
		AmmoName->SetText(FText::FromString(WeaponName(Weapon)));
		HealthText->SetColorAndOpacity(Character->GetHealth() <= 25 ? Accent : Paper);
		AmmoText->SetText(FText::FromString(Weapon && Combat ? FString::Printf(TEXT("%d / %d"),Weapon->GetAmmo(),Combat->GetCarriedAmmo()) : TEXT("未装备武器")));
		const int32 MedicalCount = Inventory ? Inventory->Count(ESupplyKind::Medical) : 0;
		const int32 SpeedCount = Inventory ? Inventory->Count(ESupplyKind::Speed) : 0;
		MedicalButton->SetIsEnabled(MedicalCount > 0 && Character->GetHealth()<100 && !Character->IsEliminated());
		SpeedButton->SetIsEnabled(SpeedCount > 0 && !Character->IsEliminated());

        PrimaryText->SetText(FText::FromString(WeaponName(Weapon)));
        SecondaryText->SetText(FText::FromString(WeaponName(Combat ? Combat->GetSecondaryWeapon() : nullptr)));
        SupplyText->SetText(FText::FromString(FString::Printf(TEXT("医疗包  %d / 3     ·     加速道具  %d / 3     ·     备弹  %d"),MedicalCount,SpeedCount,Combat ? Combat->GetCarriedAmmo() : 0)));
        Details->SetText(FText::FromString(FString::Printf(TEXT("生命 %.0f    /    击杀 %.0f    /    死亡 %d    /    成绩 %.0f"),Character->GetHealth(),Stats ? Stats->GetScore() : 0,Stats ? Stats->GetDefeats() : 0,Stats ? Stats->GetScore() : 0)));

		if (const auto* Match = GetWorld()->GetGameState<ABlasterGameState>())
		{
			const int32 Seconds = FMath::Max(0,FMath::CeilToInt(Match->GetPhaseDeadline()-Match->GetServerWorldTimeSeconds()));
			const FString Phase = Character->IsEliminated() ? TEXT("已淘汰 · 即将重返街区") : Match->GetMatchState() == MatchState::WaitingPostMatch ? TEXT("本轮结束")
				: Match->GetMatchState() == MatchState::WaitingToStart ? TEXT("准备交火") : TEXT("街区交火");
			PhaseText->SetText(FText::FromString(FString::Printf(TEXT("%s   %02d:%02d   击杀 %.0f"),*Phase,Seconds/60,Seconds%60,Stats ? Stats->GetScore() : 0)));
		}
	}
}
void UStreetWidget::Host() { GetGameInstance()->GetSubsystem<UStreetSessionSubsystem>()->Host(); }
void UStreetWidget::Join() { GetGameInstance()->GetSubsystem<UStreetSessionSubsystem>()->Join(); }
void UStreetWidget::Ready() { if (auto* PC=GetOwningPlayer<ABlasterPlayerController>()) PC->ServerRequestStart(); }
void UStreetWidget::Medical() { if (auto* Pawn=GetOwningPlayerPawn()) if (auto* Inv=Pawn->FindComponentByClass<UInventoryComponent>()) Inv->ServerUse(ESupplyKind::Medical); }
void UStreetWidget::Speed() { if (auto* Pawn=GetOwningPlayerPawn()) if (auto* Inv=Pawn->FindComponentByClass<UInventoryComponent>()) Inv->ServerUse(ESupplyKind::Speed); }
void UStreetWidget::Continue() { if (auto* PC=GetOwningPlayer<ABlasterPlayerController>()) PC->SetMenuOpen(false); }
void UStreetWidget::Leave()
{
	if (auto* PC=GetOwningPlayer<ABlasterPlayerController>(); PC && PC->IsStartScreen()) UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);
	else GetGameInstance()->GetSubsystem<UStreetSessionSubsystem>()->Leave();
}
void UStreetWidget::Settings() { SettingsPanel->SetVisibility(SettingsPanel->GetVisibility()==ESlateVisibility::Collapsed ? ESlateVisibility::Visible : ESlateVisibility::Collapsed); }
void UStreetWidget::SetSensitivity(float Value) { auto* Session=GetGameInstance()->GetSubsystem<UStreetSessionSubsystem>(); Session->Sensitivity=Value; Session->SaveSettings(); }
void UStreetWidget::SetVolume(float Value) { auto* Session=GetGameInstance()->GetSubsystem<UStreetSessionSubsystem>(); Session->Volume=Value; Session->SaveSettings(); }

int32 UStreetWidget::NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& CullRect,
    FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool bParentEnabled) const
{
    const int32 BaseLayer = Super::NativePaint(Args, Geometry, CullRect, Elements, Layer, Style, bParentEnabled);
    const auto* PC = GetOwningPlayer<ABlasterPlayerController>();
    auto* Character = Cast<ABlasterCharacter>(GetOwningPlayerPawn());
    if (!PC || PC->IsMenuOpen() || PC->IsLobby() || PC->IsStartScreen()
        || !Character || Character->IsEliminated() || !Character->GetEquippedWeapon()) return BaseLayer;
    const auto* Combat = Character->FindComponentByClass<UCombatComponent>();
    if (!Combat) return BaseLayer;
    FHitResult Hit, Block;
    Combat->TraceAim(Hit);
    const bool bBlocked = Combat->IsMuzzleBlocked(Block);
    const auto* Victim = Cast<ABlasterCharacter>(Hit.GetActor());
    const bool bTarget = Victim && !Victim->IsEliminated();
    const bool bHead = bTarget && BlasterHit::IsHead(Hit.BoneName);
    const FLinearColor Color = bBlocked ? FLinearColor(1,.3f,.1f) : bHead ? FLinearColor(1,.8f,.15f)
        : bTarget ? FLinearColor(1,.3f,.3f) : FLinearColor::White;
    const FVector2D Center = Geometry.GetLocalSize() * .5;
    // Keep a physical-pixel minimum instead of shrinking to sub-pixel Canvas strokes at 640x480.
    const float Unit = FMath::Max(1.f, 1.f / FMath::Max(.1f, Geometry.GetAccumulatedLayoutTransform().GetScale()));
    const float Gap = (Character->IsAiming() ? 5.f : 9.f) + Combat->GetShotFeedback() * 3.f;
    auto Segment = [&](float X1, float Y1, float X2, float Y2, const FLinearColor& Ink)
    {
        TArray<FVector2D> Points{Center + FVector2D(X1,Y1)*Unit, Center + FVector2D(X2,Y2)*Unit};
        FSlateDrawElement::MakeLines(Elements, BaseLayer+1, Geometry.ToPaintGeometry(), Points,
            ESlateDrawEffect::None, FLinearColor(0,0,0,.9f), true, 4.f*Unit);
        FSlateDrawElement::MakeLines(Elements, BaseLayer+2, Geometry.ToPaintGeometry(), Points,
            ESlateDrawEffect::None, Ink, true, 2.f*Unit);
    };
    Segment(-Gap-7,0,-Gap,0,Color); Segment(Gap,0,Gap+7,0,Color);
    Segment(0,-Gap-7,0,-Gap,Color); Segment(0,Gap,0,Gap+7,Color);
    if (bHead)
    {
        Segment(-5,0,0,-5,Color); Segment(0,-5,5,0,Color);
        Segment(5,0,0,5,Color); Segment(0,5,-5,0,Color);
    }
    else if (bTarget) { Segment(-4,-4,-4,4,Color); Segment(4,-4,4,4,Color); }
    else Segment(-1,0,1,0,Color);
    if (bBlocked) Segment(-13,-13,13,13,Color);
    if (Combat->GetHitFeedback()>0)
    {
        const FLinearColor Confirm = Combat->WasHeadshot() ? FLinearColor(1,.8f,.15f) : FLinearColor::White;
        for (int32 SX : {-1,1}) for (int32 SY : {-1,1}) Segment(SX*19,SY*19,SX*25,SY*25,Confirm);
    }
    return BaseLayer+2;
}
