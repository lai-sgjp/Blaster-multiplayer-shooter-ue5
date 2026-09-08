#include "BlasterHUD.h"
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
	return Model == EFireModel::Projectile ? TEXT("RIFLE") : Model == EFireModel::Hitscan ? TEXT("CARBINE") : TEXT("SHOTGUN");
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
	if (!RuntimeFont)
	{
		RuntimeFont = NewObject<UFont>(this);
		RuntimeFont->FontCacheType = EFontCacheType::Runtime;
		RuntimeFont->CompositeFont = *FCoreStyle::GetDefaultFont();
	}
	const float S = FMath::Min(Canvas->ClipX / 960.f, Canvas->ClipY / 540.f);
	const float W = Canvas->ClipX / S, H = Canvas->ClipY / S;
	auto Box = [&](float X, float Y, float Width, float Height, FLinearColor Color)
	{ DrawRect(Color, X*S, Y*S, Width*S, Height*S); };
	auto Text = [&](const FString& Value, float X, float Y, float Size, FLinearColor Color)
	{ Label(Canvas, RuntimeFont, Value, Color, X*S, Y*S, 18.f*Size*S); };
	const auto* State = GetWorld()->GetGameState<ABlasterGameState>();
	const auto* Stats = PlayerOwner->GetPlayerState<ABlasterPlayerState>();
	auto* Character = Cast<ABlasterCharacter>(PlayerOwner->GetPawn());
	Box(20,20,214,66,Panel); Box(20,20,3,66,Cyan);
	Text(TEXT("ELIMINATIONS"),34,28,0.65f,Muted);
	Text(FString::Printf(TEXT("%02.0f"),Stats ? Stats->GetScore() : 0.f),34,44,1.45f,White);
	Text(FString::Printf(TEXT("DEFEATS  %02d"),Stats ? Stats->GetDefeats() : 0),100,53,0.65f,Muted);
	Box(W/2-76,16,152,70,Panel);
	if (State)
	{
		const int32 Seconds = FMath::Max(0,FMath::CeilToInt(State->GetPhaseDeadline()-State->GetServerWorldTimeSeconds()));
		Text(State->GetMatchState()==MatchState::InProgress ? TEXT("FREE FOR ALL") : State->GetMatchState()==MatchState::WaitingToStart ? TEXT("GET READY") : TEXT("ROUND COMPLETE"),W/2-59,24,0.65f,Muted);
		Text(FString::Printf(TEXT("%02d : %02d"),Seconds/60,Seconds%60),W/2-51,43,1.35f,Seconds<15 ? Orange : White);
		float BestScore=0; int32 Leaders=0; FString Leader;
		for (const APlayerState* P : State->PlayerArray)
		{
			if (!IsValid(P) || P->IsOnlyASpectator()) continue;
			if (P->GetScore()>BestScore) { BestScore=P->GetScore(); Leader=P->GetPlayerName().Left(18); Leaders=1; }
			else if (BestScore>0 && P->GetScore()==BestScore) ++Leaders;
		}
		if (BestScore>0) Text(Leaders>1 ? TEXT("LEAD TIED") : TEXT("LEADER  ")+Leader,24,94,0.65f,Gold);
		if (State->GetServerWorldTimeSeconds()<State->GetAnnouncementDeadline())
		{
			Box(W/2-220,105,440,30,Panel);
			Text(State->GetEliminationMessage().Left(56),W/2-209,113,0.65f,Gold);
		}
	}
	const float Ping=Stats ? Stats->GetPingInMilliseconds() : 0.f;
	Box(W-190,20,170,38,Panel);
	Text(FString::Printf(TEXT("%s  %.0f ms"),Ping>150 ? TEXT("HIGH PING") : TEXT("NETWORK"),Ping),W-178,32,0.65f,Ping>150 ? Orange : Muted);
	if (!Character) return;
	DrawPickupLabels(Character,S);
	const float Health=Character->GetHealth();
	Box(20,H-100,224,80,Panel); Box(20,H-100,3,80,Health<=25 ? Orange : Cyan);
	Text(TEXT("VITALS"),34,H-90,0.65f,Muted);
	Text(FString::Printf(TEXT("%.0f"),Health),34,H-72,1.5f,Health<=25 ? Orange : White);
	Text(TEXT("/ 100 HP"),120,H-57,0.65f,Muted);
	Box(34,H-35,196,5,FLinearColor(0.1f,0.17f,0.23f));
	Box(34,H-35,196*FMath::Clamp(Health/100.f,0.f,1.f),5,Health<=25 ? Orange : Cyan);
	Text(TEXT("E  PICK UP     Q  SWAP     R  RELOAD"),W/2-119,H-28,0.60f,Muted);
	if (Character->IsEliminated())
	{
		Box(W/2-155,H/2-40,310,70,Panel);
		Text(TEXT("ELIMINATED"),W/2-78,H/2-29,1.2f,Orange);
		Text(TEXT("Returning to the arena..."),W/2-91,H/2+4,0.70f,White);
		CrosshairSpread=0; bTargetCharacter=false; return;
	}
	AWeapon* Weapon=Character->GetEquippedWeapon();
	const auto* Combat=Character->FindComponentByClass<UCombatComponent>();
	if (!IsValid(Weapon) || !Combat) { CrosshairSpread=0; bTargetCharacter=false; return; }
	const EFireModel Model=Weapon->GetFireModel();
	const FLinearColor Accent=WeaponColor(Model);
	Box(W-258,H-150,238,130,Panel); Box(W-258,H-150,238,3,Accent);
	Text(WeaponName(Model),W-243,H-139,0.95f,Accent);
	Text(FString::Printf(TEXT("%d MAG / %.2fs"),Weapon->GetMagazineCapacity(),Weapon->GetFireInterval()),W-243,H-113,0.60f,Muted);
	Text(Model==EFireModel::Projectile ? TEXT("TRAVEL TIME") : Model==EFireModel::Hitscan ? TEXT("INSTANT HIT") : TEXT("8 PELLETS / 2 DEG"),W-243,H-87,0.60f,Muted);
	Text(FString::Printf(TEXT("%02d"),Weapon->GetAmmo()),W-243,H-70,1.8f,Weapon->GetAmmo()<=5 ? Orange : White);
	Text(FString::Printf(TEXT("/ %03d"),Combat->GetCarriedAmmo()),W-161,H-51,0.85f,Muted);
	if (Combat->IsReloading()) Text(TEXT("RELOADING"),W-113,H-139,0.65f,Gold);
	else if (Weapon->GetAmmo()==0) Text(TEXT("EMPTY"),W-80,H-139,0.65f,Orange);
	FHitResult Hit; Combat->TraceAim(Hit);
	bTargetCharacter=Hit.GetActor() && Hit.GetActor()->IsA<ABlasterCharacter>();
	const float Kick=Combat->GetShotFeedback()*5.f*S;
	const FLinearColor Reticle=bTargetCharacter ? Orange : FMath::Lerp(Accent,White,FMath::Min(1.f,Combat->GetShotFeedback()));
	const float X=Canvas->ClipX/2, Y=Canvas->ClipY/2;
	CrosshairSpread=(Character->IsAiming() ? 3.f : 7.f)*S+Kick;
	auto Line=[&](float AX,float AY,float BX,float BY) { DrawLine(X+AX,Y+AY,X+BX,Y+BY,Reticle,FMath::Max(1.f,1.5f*S)); };
	if (Model==EFireModel::Shotgun)
	{
		const float FOV=Character->GetFollowCamera()->FieldOfView;
		CrosshairSpread=Canvas->ClipX*0.5f*FMath::Tan(FMath::DegreesToRadians(2.f))/FMath::Tan(FMath::DegreesToRadians(FOV/2.f));
		for (int32 I=0; I<48; ++I)
		{
			const float A=2.f*PI*I/48, B=2.f*PI*(I+1)/48;
			Line(FMath::Cos(A)*CrosshairSpread,FMath::Sin(A)*CrosshairSpread,FMath::Cos(B)*CrosshairSpread,FMath::Sin(B)*CrosshairSpread);
		}
		Line(-2*S,0,2*S,0); Line(0,-2*S,0,2*S);
		const float R=CrosshairSpread+5*S+Kick;
		Line(-R,-3*S,-R,3*S); Line(R,-3*S,R,3*S);
	}
	else if (Model==EFireModel::Hitscan)
	{
		DrawRect(Reticle,X-S,Y-S,2*S,2*S);
		Line(-11*S-Kick,-4*S,-11*S-Kick,4*S); Line(11*S+Kick,-4*S,11*S+Kick,4*S);
	}
	else
	{
		const float R=CrosshairSpread;
		Line(-R-8*S,0,-R,0); Line(R,0,R+8*S,0); Line(0,-R-8*S,0,-R); Line(0,R,0,R+8*S); Line(-2*S,0,2*S,0);
	}
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
		Labels.Add({*It,Kind==EBlasterPickupKind::Health ? TEXT("HEALTH +25") : Kind==EBlasterPickupKind::Ammo ? TEXT("AMMO +30") : TEXT("SPEED / 8 SEC"),
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
		const FString Hint=Item.Distance<180 ? (Item.bWeapon ? TEXT("[E] PICK UP") : TEXT("WALK TO COLLECT")) : FString::Printf(TEXT("%.0f m"),Item.Distance/100);
		Label(Canvas,RuntimeFont,Hint,White,Screen.X-63*Scale,Screen.Y+12*Scale,10.f*Scale);
	}
}
