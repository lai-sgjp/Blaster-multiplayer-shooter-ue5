#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "StreetSessionSubsystem.generated.h"

UCLASS()
class BLASTER_API UStreetSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void Host();
	void Join();
	void Leave();
	FString Notice;
	bool bBusy = false;
	float Sensitivity = 1.f;
	float Volume = 0.7f;
	void SaveSettings();
private:
	UPROPERTY() TObjectPtr<class UMultiplayerSessionsSubsystem> Sessions;
	UFUNCTION() void Created(bool bSuccess);
	UFUNCTION() void Destroyed(bool bSuccess);
	void Found(const TArray<FOnlineSessionSearchResult>& Results, bool bSuccess);
	void Joined(EOnJoinSessionCompleteResult::Type Result);
	void NetworkFailure(UWorld* World, UNetDriver* Driver, ENetworkFailure::Type Type, const FString& Error);
	void TravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error);
	void ReturnHome();
	FDelegateHandle FailureHandle, TravelHandle, FoundHandle, JoinedHandle;
	bool bLeaving = false;
	FString ReturnNotice;
};
