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

private:
	void SessionCreationRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGuid SessionSearchId);
	void StartFindingCreatedSession(const FGuid& SessionSearchId);
	void StopAllSessionFindings();
	void StopFindingCreatedSession();
	void StopGlobalSessionSearch();
	void FindGlobalSessions();
	void GlobalSessionSearchCompleted(bool bWasSuccessful);

	FTimerHandle FindCreatedSessionTimerHandle;
	FTimerHandle FindCreatedSessionTimeoutTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float FindCreatedSessionSearchInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float FindCreatedSessionTimeoutDuration = 120;

	FTimerHandle GlobalSessionSearchTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float GlobalSessionSearchInterval = 3.f;

	

	void FindCreatedSession(FGuid SessionSearchId);
	void FindCreatedSessionTimeout();
	void FindCreateSessionCompleted(bool bWasSuccessful);

	void JoinSessionWithSearchResult(const class FOnlineSessionSearchResult& SearchResult);
	void JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult, int Port);

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;



/*********************************/
/*          Session Server       */
/*********************************/
public:
	void PlayerJoined(const FUniqueNetIdRepl& UniqueId);
	void PlayerLeft(const FUniqueNetIdRepl& UniqueId);

	bool bReturnToMultiplayerPage = false;

	// Called when the lobby leaves team selection — pulls the session out of
	// search results so nobody can join a match that's past team picking
	void SetSessionJoinable(bool bJoinable);


private:
	void CreateSession();
	void OnSessionCreated(FName SessionName, bool bWasSuccessful);

	FString ServerSessionName;
	int SessionServerPort;

	void TerminateSessionServer();
	void EndSessionCompleted(FName SessionName, bool bWasSuccessful);

	FTimerHandle WaitPlayerJoinTimeoutHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session")
	float WaitPlayerJoinTimeoutDuration = 300.f;

	void WaitPlayerJoinTimeoutReached();

	TSet<FUniqueNetIdRepl> PlayerRecord;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> LobbyLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> Lvl_ThirdPerson;

	void LoadLevelAndListen(TSoftObjectPtr<UWorld>Level);


/*********************************/
/*         Practice Arena        */
/*********************************/

public:
	// Practice arena: solo play against AI, no session or coordinator involved
	void StartPracticeArena();

	// Lives here because it must survive the travel into the arena.
	bool bPracticeMode = false;

};
