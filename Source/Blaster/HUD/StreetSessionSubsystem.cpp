#include "StreetSessionSubsystem.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ConfigCacheIni.h"
#include "AudioDevice.h"

void UStreetSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UMultiplayerSessionsSubsystem>();
	Sessions = GetGameInstance()->GetSubsystem<UMultiplayerSessionsSubsystem>();
	Sessions->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::Created);
	Sessions->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::Destroyed);
	FoundHandle = Sessions->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::Found);
	JoinedHandle = Sessions->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::Joined);
	FailureHandle = GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::NetworkFailure);
	TravelHandle = GEngine->OnTravelFailure().AddUObject(this, &ThisClass::TravelFailure);
	GConfig->GetFloat(TEXT("Street"), TEXT("Sensitivity"), Sensitivity, GGameUserSettingsIni);
	GConfig->GetFloat(TEXT("Street"), TEXT("Volume"), Volume, GGameUserSettingsIni);
	Sensitivity = FMath::Clamp(Sensitivity, 0.2f, 3.f);
	Volume = FMath::Clamp(Volume, 0.f, 1.f);
}
void UStreetSessionSubsystem::Deinitialize()
{
	if (Sessions)
	{
		Sessions->MultiplayerOnCreateSessionComplete.RemoveAll(this);
		Sessions->MultiplayerOnDestroySessionComplete.RemoveAll(this);
		Sessions->MultiplayerOnFindSessionsComplete.Remove(FoundHandle);
		Sessions->MultiplayerOnJoinSessionComplete.Remove(JoinedHandle);
	}
	GEngine->OnNetworkFailure().Remove(FailureHandle);
	GEngine->OnTravelFailure().Remove(TravelHandle);
	Super::Deinitialize();
}
void UStreetSessionSubsystem::Host()
{
	if (bBusy) return;
	IOnlineSubsystem* Service = Online::GetSubsystem(GetWorld());
	if (!Sessions || !Service || !Service->GetSessionInterface().IsValid()) { Notice = TEXT("在线会话服务不可用，请检查 Steam。"); return; }
	if (!Online::GetSubsystem(GetWorld())) { Notice = TEXT("在线服务不可用，请启动 Steam 后重试。"); return; }
	bBusy = true; Notice = TEXT("正在创建房间…");
	Sessions->CreateSession(8, TEXT("StreetFFA"));
}
void UStreetSessionSubsystem::Join()
{
	if (bBusy) return;
	IOnlineSubsystem* Service = Online::GetSubsystem(GetWorld());
	if (!Sessions || !Service || !Service->GetSessionInterface().IsValid()) { Notice = TEXT("在线会话服务不可用，请检查 Steam。"); return; }
	if (!Online::GetSubsystem(GetWorld())) { Notice = TEXT("在线服务不可用，请启动 Steam 后重试。"); return; }
	bBusy = true; Notice = TEXT("正在寻找街区房间…");
	Sessions->FindSessions(100);
}
void UStreetSessionSubsystem::Created(bool bSuccess)
{
	if (!bBusy || bLeaving) return;
	bBusy = false;
	if (!bSuccess) { Notice = TEXT("创建失败，请检查网络后重试。"); return; }
	Notice.Empty();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Street/Maps/StreetLobby"), true, TEXT("listen"));
}
void UStreetSessionSubsystem::Found(const TArray<FOnlineSessionSearchResult>& Results, bool bSuccess)
{
	if (!bBusy || bLeaving) return;
	if (bSuccess) for (const auto& Result : Results)
	{
		FString Type;
		Result.Session.SessionSettings.Get(FName(TEXT("MatchType")), Type);
		if (Type == TEXT("StreetFFA") && Result.Session.NumOpenPublicConnections > 0)
		{
			Notice = TEXT("正在加入房间…"); Sessions->JoinSession(Result); return;
		}
	}
	bBusy = false; Notice = TEXT("暂无可加入的房间。你可以创建一个房间。");
}
void UStreetSessionSubsystem::Joined(EOnJoinSessionCompleteResult::Type Result)
{
	if (!bBusy || bLeaving) return;
	bBusy = false;
	FString Address;
	IOnlineSubsystem* OnlineService = Online::GetSubsystem(GetWorld());
	const IOnlineSessionPtr Interface = OnlineService ? OnlineService->GetSessionInterface() : nullptr;
	if (Result != EOnJoinSessionCompleteResult::Success || !Interface.IsValid()
		|| !Interface->GetResolvedConnectString(NAME_GameSession, Address))
	{ Notice = TEXT("无法加入，房间可能已满或已关闭。"); return; }
	if (auto* PC = GetGameInstance()->GetFirstLocalPlayerController()) PC->ClientTravel(Address, TRAVEL_Absolute);
}
void UStreetSessionSubsystem::Leave()
{
	if (bLeaving) return;
	bLeaving = bBusy = true;
	Notice = TEXT("正在离开房间…");
	IOnlineSubsystem* OnlineService = Online::GetSubsystem(GetWorld());
	const IOnlineSessionPtr Interface = OnlineService ? OnlineService->GetSessionInterface() : nullptr;
	if (Interface.IsValid() && Interface->GetNamedSession(NAME_GameSession)) Sessions->DestroySession();
	else ReturnHome();
}
void UStreetSessionSubsystem::Destroyed(bool bSuccess)
{
	if (!bLeaving) return;
	if (!bSuccess && ReturnNotice.IsEmpty()) ReturnNotice = TEXT("会话清理失败，已返回开始界面。");
	ReturnHome();
}
void UStreetSessionSubsystem::ReturnHome()
{
	bLeaving = bBusy = false;
	Notice = ReturnNotice; ReturnNotice.Empty();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Street/Maps/StreetStart"));
}
void UStreetSessionSubsystem::NetworkFailure(UWorld* World, UNetDriver* Driver, ENetworkFailure::Type Type, const FString& Error)
{
	if (World != GetWorld()) return;
	ReturnNotice = TEXT("与房间的连接已断开，房主可能已离开。");
	Leave();
}
void UStreetSessionSubsystem::TravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error)
{
	if (World != GetWorld()) return;
	ReturnNotice = TEXT("场景加载失败，请重新加入房间。");
	Leave();
}
void UStreetSessionSubsystem::SaveSettings()
{
	GConfig->SetFloat(TEXT("Street"), TEXT("Sensitivity"), Sensitivity, GGameUserSettingsIni);
	GConfig->SetFloat(TEXT("Street"), TEXT("Volume"), Volume, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
	if (GetWorld()) if (auto* Device = GetWorld()->GetAudioDeviceRaw()) Device->SetTransientPrimaryVolume(Volume);
}
