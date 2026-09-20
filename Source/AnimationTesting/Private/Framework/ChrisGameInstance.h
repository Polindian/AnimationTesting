// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "ChrisGameInstance.generated.h"


DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnLoginCompleted, bool /*bWasSuccessful*/, const FString& /*PlayerNickname*/, const FString& /*ErrorMessage*/);
DECLARE_MULTICAST_DELEGATE(FOnJoinSessionFailed);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGlobalSessionSearchCompleted, const TArray<FOnlineSessionSearchResult>& /*SearchResults*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTravelFailedWithReason, const FString& /*Reason*/);

/**
 *
 */
UCLASS()
class UChrisGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void StartMatch();
	virtual void Init() override;

	// Survives travel, so the same hint doesn't come up twice running
	int32 LastHintIndex = -1;


	/*********************************/
	/*             Login             */
	/*********************************/
public:
	bool IsLoggedIn() const;
	bool IsLoggingIn() const;
	void ClientAccountPortalLogin();
	void ClientSteamLogin();
	FOnLoginCompleted OnLoginCompleted;

	void ClientDevAuthLogin(const FString& CredentialName);

private:
	void ClientLogin(const FString& Type, const FString& Id, const FString& Token);
	void LoginCompleted(int NumOfLocalPlayers, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorMessage);

	FDelegateHandle LoggingInDelegateHandle;

	/*********************************/
	/*         Client Session        */
	/*********************************/

public:
	void RequestCreateAndJoinSession(const FName& NewSessionName);
	void CancelSessionCreation();
	void StartGlobalSessionSearch();

	bool JoinSessionWithId(const FString& SessionIdString);

	FOnJoinSessionFailed OnJoinSessionFailed;
	FOnGlobalSessionSearchCompleted OnGlobalSessionSearchCompleted;

	// A PreLogin rejection or a dead server surfaces here, not in OnJoinSessionFailed —
	// the EOS join has already succeeded by the time the connection is refused
	FOnTravelFailedWithReason OnTravelFailedWithReason;

private:
	void SessionCreationRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGuid SessionSearchId);
	void StartFindingCreatedSession(const FGuid& SessionSearchId);
	void StopAllSessionFindings();
	void StopFindingCreatedSession();
	void StopGlobalSessionSearch();
	void FindGlobalSessions();
	void GlobalSessionSearchCompleted(bool bWasSuccessful);

	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
		ENetworkFailure::Type FailureType, const FString& ErrorString);

	FTimerHandle FindCreatedSessionTimerHandle;
	FTimerHandle FindCreatedSessionTimeoutTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float FindCreatedSessionSearchInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float FindCreatedSessionTimeoutDuration = 120;

	FTimerHandle GlobalSessionSearchTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float GlobalSessionSearchInterval = 8.f;

	// A server that stops responding mid-join produces no callback at all,
	// so the waiting widget needs its own way out
	FTimerHandle JoinSessionTimeoutHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float JoinSessionTimeoutDuration = 30.f;

	void JoinSessionTimeout();

	void FindCreatedSession(FGuid SessionSearchId);
	void FindCreatedSessionTimeout();
	void FindCreateSessionCompleted(bool bWasSuccessful);

	void JoinSessionWithSearchResult(const class FOnlineSessionSearchResult& SearchResult);
	void JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult, int Port);

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	// The player controller's BeginPlay runs ~2s after seamless travel finishes,
	// so the arena renders before any controller-side cover can exist
	void HandlePostLoadMap(UWorld* LoadedWorld);

	UPROPERTY(EditDefaultsOnly, Category = "Travel")
	TSubclassOf<class UUserWidget> TravelCoverWidgetClass;

	UPROPERTY(Transient)
	class UUserWidget* TravelCoverWidget = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Travel")
	float TravelCoverDuration = 5.f;

	FTimerHandle TravelCoverTimerHandle;

	/*********************************/
	/*          Session Server       */
	/*********************************/
public:
	void PlayerJoined(const FUniqueNetIdRepl& UniqueId);
	void PlayerLeft(const FUniqueNetIdRepl& UniqueId);

	bool bReturnToMultiplayerPage = false;
	// Set by whoever sent the player back; the rebuilt menu widget shows it.
	// Empty means no dialog.
	FText PendingMenuMessage;

	// Called when the lobby leaves team selection — pulls the session out of
	// search results so nobody can join a match that's past team picking
	void SetSessionJoinable(bool bJoinable);

	// Flask decides what's joinable; EOS keeps advertising sessions it shouldn't
	bool IsSessionJoinable(const FString& SessionSearchId) const
	{
		return JoinableSessionIds.Contains(SessionSearchId);
	}

	// EOS keeps the client registered in the session after travelling away, and
	// the next JoinSession fails until it's destroyed
	void LeaveCurrentSession();

	void TerminateSessionServer();

	void CancelSessionJoin();

	bool bJoinCancelled = false;

	FString PendingCreateSearchId;

private:
	void CreateSession();
	void OnSessionCreated(FName SessionName, bool bWasSuccessful);

	FString ServerSessionName;
	int SessionServerPort;

	
	void EndSessionCompleted(FName SessionName, bool bWasSuccessful);

	FTimerHandle WaitPlayerJoinTimeoutHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session")
	float WaitPlayerJoinTimeoutDuration = 100.f;

	void WaitPlayerJoinTimeoutReached();

	TSet<FUniqueNetIdRepl> PlayerRecord;

	FString PendingJoinSessionId;
	void DestroyBeforeJoinCompleted(FName SessionName, bool bWasSuccessful);

	FName JoinedSessionName;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> LobbyLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> Lvl_ThirdPerson;

	void LoadLevelAndListen(TSoftObjectPtr<UWorld>Level);

	// EOS keeps advertising despite bShouldAdvertise, so the coordinator is what
	// actually controls whether this session appears in the browser.
	// Status is "open", "started" or "ended".
	void ReportSessionStatusToCoordinator(const FString& Status);

	// Silence means dead, whatever the cause — this survives crashes and hard
	// kills, which a shutdown-time report cannot
	FTimerHandle HeartbeatTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session")
	float HeartbeatInterval = 30.f;

	void SendHeartbeat();

	TSet<FString> JoinableSessionIds;

	void FetchJoinableSessions();
	void JoinableSessionsFetched(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);


	/*********************************/
	/*         Practice Arena        */
	/*********************************/

public:
	// Practice arena: solo play against AI, no session or coordinator involved
	void StartPracticeArena();

	// Lives here because it must survive the travel into the arena.
	bool bPracticeMode = false;

};