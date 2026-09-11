#include "BlasterHUD.h"
#include "Blaster/Character/BlasterPlayerController.h"
#include "Blaster/BlasterComponent/BlasterHitRules.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Character/BlasterPlayerState.h"
#include "Blaster/Character/BlasterGameState.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Pickups/BlasterPickup.h"
#include "Camera/CameraComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "Styling/CoreStyle.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameMode.h"

namespace
{
const FLinearColor White(0.91f, 0.95f, 1.f), Muted(0.48f, 0.59f, 0.70f), Panel(0.012f, 0.025f, 0.047f, 0.88f);
const FLinearColor Cyan(0.15f, 0.78f, 1.f), Gold(1.f, 0.73f, 0.23f), Orange(1.f, 0.36f, 0.16f);
const TCHAR* WeaponName(EFireModel Model)
{
	return Model == EFireModel::Projectile ? TEXT("突击步枪") : Model == EFireModel::Hitscan ? TEXT("卡宾枪") : TEXT("霰弹枪");
}
FLinearColor WeaponColor(EFireModel Model)
{
	return Model == EFireModel::Projectile ? Gold : Model == EFireModel::Hitscan ? Cyan : Orange;
}
void Label(UCanvas* Canvas, UFont* Font, const FString& Value, const FLinearColor& Color, float X, float Y, float Size)
{
	FCanvasTextItem Item(FVector2D(X,Y), FText::FromString(Value),
		FSlateFontInfo(Font, FMath::Max(9, FMath::RoundToInt(Size))), Color);
	Canvas->DrawItem(Item);
}
}

void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !PlayerOwner) return;
    const auto* PC = Cast<ABlasterPlayerController>(PlayerOwner);
    if (PC && (PC->IsMenuOpen() || PC->IsLobby() || PC->IsStartScreen())) return;
    auto* Character = Cast<ABlasterCharacter>(PlayerOwner->GetPawn());
    if (!Character || Character->IsEliminated()) return;
    if (!RuntimeFont)
    {
        RuntimeFont = NewObject<UFont>(this);
        RuntimeFont->FontCacheType = EFontCacheType::Runtime;
        FTypefaceEntry Entry(FName(TEXT("Bold")));
        Entry.Font = FFontData(LoadObject<UObject>(nullptr, TEXT("/Game/Street/UI/StreetFont.StreetFont")));
        RuntimeFont->CompositeFont.DefaultTypeface.Fonts.Add(Entry);
    }
    const float S = Canvas->ClipY / 1080.f;
    DrawPickupLabels(Character, S);

}

void ABlasterHUD::DrawPickupLabels(ABlasterCharacter* Character,float Scale)
{
	if (Character->IsAiming() || Character->IsEliminated()) return;
	struct FLabel { AActor* Actor; FString Name; FLinearColor Color; double Distance; bool bWeapon; };
	TArray<FLabel> Labels;
	for (TActorIterator<AWeapon> It(GetWorld()); It; ++It)
		if (!It->GetOwner() && !It->IsHidden()) Labels.Add({*It,WeaponName(It->GetFireModel()),WeaponColor(It->GetFireModel()),FVector::Dist(Character->GetActorLocation(),It->GetActorLocation()),true});
	for (TActorIterator<ABlasterPickup> It(GetWorld()); It; ++It)
	{
		if (It->IsHidden()) continue;
		const auto Kind=It->GetKind();
		Labels.Add({*It,Kind==EBlasterPickupKind::Health ? TEXT("医疗包 +1") : Kind==EBlasterPickupKind::Ammo ? TEXT("弹药 +30") : TEXT("加速道具 +1"),
			Kind==EBlasterPickupKind::Health ? FLinearColor(0.25f,1.f,0.5f) : Kind==EBlasterPickupKind::Ammo ? Gold : Cyan,
			FVector::Dist(Character->GetActorLocation(),It->GetActorLocation()),false});
	}
	Labels.Sort([](const FLabel& A,const FLabel& B) { return A.Distance<B.Distance; });
	int32 Shown=0; TArray<FVector2D> Used;
	for (const FLabel& Item : Labels)
	{
		if (Shown>=6 || Item.Distance>1200) break;
		const FVector World=Item.Actor->GetActorLocation()+FVector(0,0,55);
		FVector2D Screen;
		if (!PlayerOwner->ProjectWorldLocationToScreen(World,Screen,true) || Screen.Y<145*Scale || Screen.Y>Canvas->ClipY-130*Scale || Screen.X<85*Scale || Screen.X>Canvas->ClipX-85*Scale) continue;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(PickupLabel),false,Character); Params.AddIgnoredActor(Item.Actor);
		FHitResult Block;
		if (GetWorld()->LineTraceSingleByChannel(Block,Character->GetFollowCamera()->GetComponentLocation(),World,ECC_Visibility,Params)) continue;
		bool bOverlap=false;
		for (const auto& Position : Used) if (FMath::Abs(Position.X-Screen.X)<145*Scale && FMath::Abs(Position.Y-Screen.Y)<47*Scale) bOverlap=true;
		if (bOverlap) continue;
		Used.Add(Screen); ++Shown;
		DrawRect(Panel,Screen.X-72*Scale,Screen.Y-10*Scale,144*Scale,40*Scale);
		DrawRect(Item.Color,Screen.X-72*Scale,Screen.Y-10*Scale,2*Scale,40*Scale);
		Label(Canvas,RuntimeFont,Item.Name,Item.Color,Screen.X-63*Scale,Screen.Y-6*Scale,12.f*Scale);
		const FString Hint=Item.Distance<180 ? (Item.bWeapon ? TEXT("[E] 拾取") : TEXT("靠近拾取")) : FString::Printf(TEXT("%.0f m"),Item.Distance/100);
		Label(Canvas,RuntimeFont,Hint,White,Screen.X-63*Scale,Screen.Y+12*Scale,10.f*Scale);
	}
}
