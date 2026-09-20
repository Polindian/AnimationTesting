// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameInstance.h"
#include "ChrisGameInstance.h"
#include "Network/ChrisNetStatics.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Blueprint/UserWidget.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "Engine/World.h"

// Only the server (dedicated or listen) should initiate a map travel
void UChrisGameInstance::StartMatch()
{
	bPracticeMode = false;
	
	const ENetMode NetMode = GetWorld()->GetNetMode();

	if (NetMode == ENetMode::NM_DedicatedServer || NetMode == ENetMode::NM_ListenServer)
	{
		LoadLevelAndListen(Lvl_ThirdPerson);
		return;
	}

#if WITH_EDITOR
	// PIE-only: solo standalone has no server net mode, so travel plainly with no
	// ?listen and no port. Compiled out of packaged builds entirely.
	if (NetMode == ENetMode::NM_Standalone && GIsEditor)
	{
		const FName LevelURL = FName(*FPackageName::ObjectPathToPackageName(Lvl_ThirdPerson.ToString()));
		if (LevelURL != NAME_None)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EDITOR] Solo travel to: %s"), *LevelURL.ToString());
			GetWorld()->ServerTravel(LevelURL.ToString());
		}
	}
#endif
}

void UChrisGameInstance::Init()
{
	Super::Init();

	if (GetWorld()->IsEditorWorld())
		return;

	if (UChrisNetStatics::IsSessionServer(this))
	{
		CreateSession();
	}

	// PreLogin rejections land here, not in OnJoinSessionFailed — the EOS join
	// already succeeded by the time the server refuses the connection
	GEngine->OnNetworkFailure().AddUObject(this, &UChrisGameInstance::HandleNetworkFailure);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UChrisGameInstance::HandlePostLoadMap);
}

bool UChrisGameInstance::IsLoggedIn() const
{
	// Fetch Identiy Interface from OSS module 
	if (IOnlineIdentityPtr IdentityPtr = UChrisNetStatics::GetIdentityPtr())
	{
		// '0' = local player index. Only have one local player 
		return IdentityPtr->GetLoginStatus(0) == ELoginStatus::LoggedIn;
	}
	return false;
}

// Only hold a valid handle while a login request is in flight - allows UI to react later whilst logging in
bool UChrisGameInstance::IsLoggingIn() const
{
	return LoggingInDelegateHandle.IsValid();
}

void UChrisGameInstance::ClientAccountPortalLogin()
{
	ClientLogin("AccountPortal", "", "");
}

// EOSPlus logs into Steam first, then EOS Connect using Steam's ticket, so there are no credentials to pass
void UChrisGameInstance::ClientSteamLogin()
{
	ClientLogin(TEXT(""), TEXT(""), TEXT(""));
}

void UChrisGameInstance::ClientDevAuthLogin(const FString& CredentialName)
{
	ClientLogin("Developer", "localhost:6547", CredentialName);
}

void UChrisGameInstance::ClientLogin(const FString& Type, const FString& Id, const FString& Token)
{
	if (IOnlineIdentityPtr IdentityPtr = UChrisNetStatics::GetIdentityPtr())
	{
		// SAFETY: if a previous login attempt somehow left a subscription behind (double-click, earlier attempt still pending), unsubscribe it first
		if (LoggingInDelegateHandle.IsValid())
		{
			IdentityPtr->OnLoginCompleteDelegates->Remove(LoggingInDelegateHandle);
			LoggingInDelegateHandle.Reset();
		}

		// SUBSCRIBE before we ask for login. AddUObject returns a handle for our proof of subscription, needed later to unsubscribe. 
		LoggingInDelegateHandle = IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UChrisGameInstance::LoginCompleted);

		// Fire the actual request. The bool returned here is "the request was accepted and is now in flight".
		if (!IdentityPtr->Login(0, FOnlineAccountCredentials(Type, Id, Token)))
		{
			UE_LOG(LogTemp, Warning, TEXT("Login failed right away!"));

			// If failed we must clean up the subscription ourselves 
			if (LoggingInDelegateHandle.IsValid())
			{
				IdentityPtr->OnLoginCompleteDelegates->Remove(LoggingInDelegateHandle);
				LoggingInDelegateHandle.Reset();
			}

			// Tell the UI it failed, so the button re-enables etc.
			OnLoginCompleted.Broadcast(false, "", "Login failed right away!");
		}
	}
}

// The callback EOS invokes when the async login finishes (success OR failure).
void UChrisGameInstance::LoginCompleted(int NumOfLocalPlayers, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorMessage)
{
	if (IOnlineIdentityPtr IdentityPtr = UChrisNetStatics::GetIdentityPtr())
	{
		// Unsubscribe immediately 
		if (LoggingInDelegateHandle.IsValid())
		{
			IdentityPtr->OnLoginCompleteDelegates->Remove(LoggingInDelegateHandle);
			LoggingInDelegateHandle.Reset();
		}

		FString PlayerNickname = "";
		if (bWasSuccessful)
		{
			PlayerNickname = IdentityPtr->GetPlayerNickname(UserId);
			UE_LOG(LogTemp, Warning, TEXT("Logged in successfull as: %s"), *(PlayerNickname));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Login failed: %s"), *(ErrorMessage));
		}

		OnLoginCompleted.Broadcast(bWasSuccessful, PlayerNickname, ErrorMessage);
	}
	else
	{
		// Re-broadcast the result on OUR OWN delegate
		OnLoginCompleted.Broadcast(false, "", "Cannot find Identity Pointer!");
	}
}

void UChrisGameInstance::RequestCreateAndJoinSession(const FName& NewSessionName)
{
	UE_LOG(LogTemp, Warning, TEXT("Requesting create and join session: %s"), *(NewSessionName.ToString()));

	// Coordinator is our own web service (not EOS) so it is talked to over HTTP
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	// Client invents the search id up front: coordinator passes it to the launched server, server advertises it, and we search EOS for it to find OUR server
	FGuid SessionSearchId = FGuid::NewGuid();

	// Local 
	FString CoordinatorURL = UChrisNetStatics::GetCoordinatorURLString();

	// POST {base}/Session = "create a session" in web API convention
	FString URL = FString::Printf(TEXT("%s/Session"), *CoordinatorURL);
	UE_LOG(LogTemp, Warning, TEXT("Sending request session creation to URL: %s"), *CoordinatorURL);
	Request->SetURL(URL);
	Request->SetVerb("POST");

	// Declares the body format
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Build {"SESSION_NAME": ..., "SESSION_SEARCH_ID": ...} — same key strings the coordinator turns into the server's launch args 
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(UChrisNetStatics::GetSessionNameKey().ToString(), NewSessionName.ToString());
	JsonObject->SetStringField(UChrisNetStatics::GetSessionSearchIdKey().ToString(), SessionSearchId.ToString());

	// Serialize: in-memory JSON object -> text form for the body
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// Async Completion
	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &UChrisGameInstance::SessionCreationRequestCompleted, SessionSearchId);

	PendingCreateSearchId = SessionSearchId.ToString(EGuidFormats::Digits);

	if(!Request->ProcessRequest())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Creation Request failed right away!"));
	}
}

// CANCEL pressed: tear down the targeted find, resume the global browse. The launched server keeps booting — WaitPlayerJoin timeout executes
void UChrisGameInstance::CancelSessionCreation()
{
	UE_LOG(LogTemp, Warning, TEXT("Cancelling session creation"));
	StopAllSessionFindings();

	if (IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr())
	{
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
		SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
	}

	if (!PendingCreateSearchId.IsEmpty())
	{
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
		Request->SetURL(UChrisNetStatics::GetCoordinatorURLString() + TEXT("/SessionCancel"));
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(FString::Printf(TEXT("{\"%s\":\"%s\"}"),
			*UChrisNetStatics::GetSessionSearchIdKey().ToString(), *PendingCreateSearchId));
		Request->ProcessRequest();

		PendingCreateSearchId.Empty();
	}

	StartGlobalSessionSearch();
}

void UChrisGameInstance::StartGlobalSessionSearch()
{
	UE_LOG(LogTemp, Warning, TEXT("Starting Global Session Search!"));
	GetWorld()->GetTimerManager().SetTimer(GlobalSessionSearchTimerHandle, this, &UChrisGameInstance::FindGlobalSessions, GlobalSessionSearchInterval, true, 0.f);
}

bool UChrisGameInstance::JoinSessionWithId(const FString& SessionIdString)
{
	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();

	// A stale local session from a previous lobby makes JoinSession fail with
	// AlreadyInSession. DestroySession is async, so join from its callback
	// rather than immediately after firing it.
	if (SessionPtr && !JoinedSessionName.IsNone() && SessionPtr->GetNamedSession(JoinedSessionName))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Session] Destroying stale session '%s' before joining"), *JoinedSessionName.ToString());

		PendingJoinSessionId = SessionIdString;

		SessionPtr->OnDestroySessionCompleteDelegates.RemoveAll(this);
		SessionPtr->OnDestroySessionCompleteDelegates.AddUObject(
			this, &UChrisGameInstance::DestroyBeforeJoinCompleted);
		SessionPtr->DestroySession(JoinedSessionName);

		return true;
	}

	if (SessionSearch.IsValid())
	{
		const FOnlineSessionSearchResult* SessionSearchResult = SessionSearch->SearchResults.FindByPredicate(
			[=](const FOnlineSessionSearchResult& Result)
			{
				return Result.GetSessionIdStr() == SessionIdString;
			}
		);

		if (SessionSearchResult)
		{
			JoinSessionWithSearchResult(*SessionSearchResult);
			return true;
		}
	}

	return false;
}

void UChrisGameInstance::SessionCreationRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGuid SessionSearchId)
{
	if(!bConnectedSuccessfully)
	{
		UE_LOG(LogTemp, Warning, TEXT("Connection failedl!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Connection was successful!"));

	// 200 = coordinator says OK
	int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode != 200)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Creation Failed with code: %d"), ResponseCode);
		return;
	}

	FString ResponseString = Response->GetContentAsString();

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
	int32 Port = 0;

	// Deserialize: response text -> JSON object -> port field
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		Port = JsonObject->GetIntegerField(*(UChrisNetStatics::GetPortKey().ToString()));
	}

	UE_LOG(LogTemp, Warning, TEXT("Connected to Coordinator Successfully and new session created is on port: %d"), Port);

	// Server was LAUNCHED, not yet READY — poll EOS until its session appears
	StartFindingCreatedSession(SessionSearchId);
}

void UChrisGameInstance::StartFindingCreatedSession(const FGuid& SessionSearchId)
{
	if (!SessionSearchId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session search ID is invalid, cannot start finding!"));
		return;
	}

	// Only one search may run at a time — clear anything in flight first
	StopAllSessionFindings();
	UE_LOG(LogTemp, Warning, TEXT("Start finding create session with Id: %s!"), *(SessionSearchId.ToString()));

	// Poller: every interval, ask EOS for the session 
	GetWorld()->GetTimerManager().SetTimer(FindCreatedSessionTimerHandle, FTimerDelegate::CreateUObject(this, &UChrisGameInstance::FindCreatedSession, SessionSearchId), FindCreatedSessionSearchInterval, true, 0.f);

	// Timeout: one-shot deadline so a dead server can't leave us searching forever
	GetWorld()->GetTimerManager().SetTimer(FindCreatedSessionTimeoutTimerHandle, this, &UChrisGameInstance::FindCreatedSessionTimeout, FindCreatedSessionTimeoutDuration);
}


// StopAllSessionFindings: searches are mutually exclusive — targeted find and (future) server-browser search must never overlap on the session interface
void UChrisGameInstance::StopAllSessionFindings()
{
	UE_LOG(LogTemp, Warning, TEXT("Stopping all session search"));

	StopFindingCreatedSession();
	StopGlobalSessionSearch();
}

// Mirror of StartFindingCreatedSession: clear BOTH timers, unsubscribe find + (preemptively) join 
void UChrisGameInstance::StopFindingCreatedSession()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop finding Created Session!"));
	GetWorld()->GetTimerManager().ClearTimer(FindCreatedSessionTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(FindCreatedSessionTimeoutTimerHandle);

	if (IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr())
	{
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
		SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
	}
}


void UChrisGameInstance::StopGlobalSessionSearch()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop global session search!"));
	if (GlobalSessionSearchTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(GlobalSessionSearchTimerHandle);
	}

	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (SessionPtr)
	{
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	}
}

void UChrisGameInstance::FindGlobalSessions()
{
	UE_LOG(LogTemp, Warning, TEXT("---- Retrying Global Session Search -----"));

	FetchJoinableSessions();

	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (!SessionPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannotfind Session Interface, wait for the next Global Session Search!"));
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 30;

	SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UChrisGameInstance::GlobalSessionSearchCompleted);

	if (!SessionPtr->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Find global session failed right away!"));
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	}
}

void UChrisGameInstance::GlobalSessionSearchCompleted(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		OnGlobalSessionSearchCompleted.Broadcast(SessionSearch->SearchResults);
		for (const FOnlineSessionSearchResult& OnlineSessionSearchResult : SessionSearch->SearchResults)
		{
			FString SessionName = "Name_None";
			OnlineSessionSearchResult.Session.SessionSettings.Get<FString>(UChrisNetStatics::GetSessionNameKey(), SessionName);
			UE_LOG(LogTemp, Warning, TEXT("Found session: %s after global session search!"), *SessionName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Global Session Search completed unsuccessfully!"));
	}

	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (SessionPtr)
	{
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	}
}

void UChrisGameInstance::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!TravelCoverWidgetClass || !LoadedWorld) { return; }

	// Only the hero selection to arena travel has the gap worth covering —
	// every other map load is fast enough not to need it
	const FString ArenaName = FPackageName::ObjectPathToPackageName(Lvl_ThirdPerson.ToString());
	if (!ArenaName.EndsWith(LoadedWorld->GetName())) { return; }

	APlayerController* PC = LoadedWorld->GetFirstPlayerController();
	if (!PC) { return; }

	TravelCoverWidget = CreateWidget<UUserWidget>(PC, TravelCoverWidgetClass);
	if (!TravelCoverWidget) { return; }

	TravelCoverWidget->AddToViewport(2000);   // above the loading screen's 1000

	LoadedWorld->GetTimerManager().SetTimer(TravelCoverTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (TravelCoverWidget)
				{
					TravelCoverWidget->RemoveFromParent();
					TravelCoverWidget = nullptr;
				}
			}), TravelCoverDuration, false);
}


void UChrisGameInstance::JoinSessionTimeout()
{
	OnTravelFailedWithReason.Broadcast(TEXT("Timed out"));
}

// One poll: build a targeted query and fire an async EOS search
void UChrisGameInstance::FindCreatedSession(FGuid SessionSearchId)
{
	UE_LOG(LogTemp, Warning, TEXT("Trying to find created session!"));
	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (!SessionPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find session ptr, cancelling session search!"));
		return;
	}

	// Member (not local): must outlive the async FindSessions call
	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	if (!SessionSearch)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot create session search, cancelling session search!"));
		return;
	}

	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 1;

	// Match exactly OUR GUID (advertised by the server as searchable metadata in GenerateOnlineSessionSettings)
	SessionSearch->QuerySettings.Set(UChrisNetStatics::GetSessionSearchIdKey(), SessionSearchId.ToString(), EOnlineComparisonOp::Equals);

	SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UChrisGameInstance::FindCreateSessionCompleted);
	if (!SessionPtr->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Find session failed right away!"));
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	}

}

void UChrisGameInstance::FindCreatedSessionTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("Created session timeout reached!"));
	StopFindingCreatedSession();
}

// 'Not Found' expected while the server boots; the poll timer retries. 'Found' means stop polling and join
void UChrisGameInstance::FindCreateSessionCompleted(bool bWasSuccessful)
{
	if (!bWasSuccessful || SessionSearch->SearchResults.Num() == 0)
	{
		return;
	}

	StopFindingCreatedSession();
	JoinSessionWithSearchResult(SessionSearch->SearchResults[0]);
}

// Read side of GenerateOnlineSessionSettings: pull the advertised name and port back out of the found session's metadata
void UChrisGameInstance::JoinSessionWithSearchResult(const FOnlineSessionSearchResult& SearchResult)
{
	bJoinCancelled = false;
	
	UE_LOG(LogTemp, Warning, TEXT("Joining session with search result!"));
	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (!SessionPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find session ptr, cancel joining"));
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(JoinSessionTimeoutHandle, this,
		&UChrisGameInstance::JoinSessionTimeout, JoinSessionTimeoutDuration, false);

	FString SessionName = "";
	SearchResult.Session.SessionSettings.Get<FString>(UChrisNetStatics::GetSessionNameKey(), SessionName);

	const FOnlineSessionSetting* PortSetting = SearchResult.Session.SessionSettings.Settings.Find(UChrisNetStatics::GetPortKey());
	int64 Port = 7777;
	if (PortSetting)
	{
		PortSetting->Data.GetValue(Port);
	}

	UE_LOG(LogTemp, Warning, TEXT("Trying to join session: %s, at port: %lld"), *(SessionName), Port);

	JoinedSessionName = FName(SessionName);

	SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
	SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UChrisGameInstance::JoinSessionCompleted, (int) Port);
	if (!SessionPtr->JoinSession(0, FName(SessionName), SearchResult))
	{
		UE_LOG(LogTemp, Warning, TEXT("Joining session failed right away!"));
		SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
		OnJoinSessionFailed.Broadcast();
	}
}

void UChrisGameInstance::JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult, int Port)
{
	if (bJoinCancelled)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Session] Join completed but was cancelled — not travelling"));
		bJoinCancelled = false;
		LeaveCurrentSession();
		return;
	}

	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();

	GetWorld()->GetTimerManager().ClearTimer(JoinSessionTimeoutHandle);

	if (!SessionPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Joining session completed, but cannot find session pointer!"));
		OnJoinSessionFailed.Broadcast();
		return;
	}

	if (JoinResult == EOnJoinSessionCompleteResult::Success)
	{
		StopAllSessionFindings();
		UE_LOG(LogTemp, Warning, TEXT("Joining session: %s successful, the port is: %lld!"), *(SessionName.ToString()), Port);

		FString TravelURL = "";
		SessionPtr->GetResolvedConnectString(SessionName, TravelURL);

		FString TestingURL = UChrisNetStatics::GetTestingURL();
		if (!TestingURL.IsEmpty())
		{
			TravelURL = TestingURL;
		}

		UChrisNetStatics::ReplacePort(TravelURL, Port);

		UE_LOG(LogTemp, Warning, TEXT("Travelling to session at: %s!"), *TravelURL);

		GetFirstLocalPlayerController(GetWorld())->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Session] Join failed, result: %d"), (int32)JoinResult);
		OnJoinSessionFailed.Broadcast();
	}

	SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
}

// TSet used so a duplicate register/unregister can't corrupt the count
void UChrisGameInstance::PlayerJoined(const FUniqueNetIdRepl& UniqueId)
{
	// Clears the timer handle once there is a player connection
	if (WaitPlayerJoinTimeoutHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(WaitPlayerJoinTimeoutHandle);
	}
	PlayerRecord.Add(UniqueId);
}

void UChrisGameInstance::PlayerLeft(const FUniqueNetIdRepl& UniqueId)
{
	PlayerRecord.Remove(UniqueId);
	if (PlayerRecord.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Server Shutdown after all players left!"));
		TerminateSessionServer(); // New termination path instead of timer handle
	}
}

void UChrisGameInstance::SetSessionJoinable(bool bJoinable)
{
	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (!SessionPtr || ServerSessionName.IsEmpty()) { return; }

	const FName SessionFName(ServerSessionName);

	FOnlineSessionSettings* CurrentSettings = SessionPtr->GetSessionSettings(SessionFName);
	if (!CurrentSettings) { return; }

	// bShouldAdvertise false removes it from EOS search results, so it drops off
	// the multiplayer page on the clients' next refresh
	FOnlineSessionSettings NewSettings = *CurrentSettings;
	NewSettings.bShouldAdvertise = bJoinable;
	NewSettings.bAllowJoinInProgress = bJoinable;

	SessionPtr->UpdateSession(SessionFName, NewSettings, true);

	UE_LOG(LogTemp, Warning, TEXT("[Session] Joinable set to %d"), bJoinable ? 1 : 0);

	ReportSessionStatusToCoordinator(bJoinable ? TEXT("open") : TEXT("started"));
}

void UChrisGameInstance::LeaveCurrentSession()
{
	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (!SessionPtr || JoinedSessionName.IsNone()) { return; }

	SessionPtr->DestroySession(JoinedSessionName);

	UE_LOG(LogTemp, Warning, TEXT("[Session] Destroyed local session '%s'"), *JoinedSessionName.ToString());

	JoinedSessionName = NAME_None;
}

void UChrisGameInstance::CreateSession()
{
	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (SessionPtr)
	{
		ServerSessionName = UChrisNetStatics::GetSessionNameString();
		FString SessionSearchId = UChrisNetStatics::GetSessionSearchIdString();
		SessionServerPort = UChrisNetStatics::GetSessionPort();

		FOnlineSessionSettings OnlineSessionSetting = UChrisNetStatics::GenerateOnlineSessionSettings(FName(ServerSessionName), *SessionSearchId, SessionServerPort);

		SessionPtr->OnCreateSessionCompleteDelegates.RemoveAll(this);
		SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UChrisGameInstance::OnSessionCreated);

		if (!SessionPtr->CreateSession(0, FName(ServerSessionName), OnlineSessionSetting))
		{
			UE_LOG(LogTemp, Warning, TEXT("Session Creation FAILED right away!"));
			SessionPtr->OnCreateSessionCompleteDelegates.RemoveAll(this);
			TerminateSessionServer();
		}

		UE_LOG(LogTemp, Warning, TEXT("Creating Session with Name: %s, SearchId: %s, Port: %d"), *ServerSessionName, *SessionSearchId, SessionServerPort);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find session pointer, terminating session!"));
		TerminateSessionServer();
	}
}
void UChrisGameInstance::OnSessionCreated(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Creation SUCCESSFUL!"));
		GetWorld()->GetTimerManager().SetTimer(WaitPlayerJoinTimeoutHandle, this, &UChrisGameInstance::WaitPlayerJoinTimeoutReached, WaitPlayerJoinTimeoutDuration);
		LoadLevelAndListen(LobbyLevel);
		GetWorld()->GetTimerManager().SetTimer(HeartbeatTimerHandle, this, &UChrisGameInstance::SendHeartbeat, HeartbeatInterval, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Creation FAILED!"));
		TerminateSessionServer();
	}

	if (IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr())
	{
		SessionPtr->OnCreateSessionCompleteDelegates.RemoveAll(this);
	}
}

void UChrisGameInstance::TerminateSessionServer()
{
	// Sent first: every path below can reach RequestExit, and a queued HTTP request won't survive that
	ReportSessionStatusToCoordinator(TEXT("ended"));

	if (IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr())
	{
		SessionPtr->OnEndSessionCompleteDelegates.RemoveAll(this);
		SessionPtr->OnEndSessionCompleteDelegates.AddUObject(this, &UChrisGameInstance::EndSessionCompleted);
		if (!SessionPtr->EndSession(FName{ ServerSessionName }))
		{
			FGenericPlatformMisc::RequestExit(false);
		}
	}
	else
	{
		FGenericPlatformMisc::RequestExit(false);
	}
}

void UChrisGameInstance::CancelSessionJoin()
{
	UE_LOG(LogTemp, Warning, TEXT("[Session] Join cancelled by user"));

	bJoinCancelled = true;

	GetWorld()->GetTimerManager().ClearTimer(JoinSessionTimeoutHandle);
	StartGlobalSessionSearch();
}

void UChrisGameInstance::EndSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	FGenericPlatformMisc::RequestExit(false);
}

void UChrisGameInstance::WaitPlayerJoinTimeoutReached()
{
	UE_LOG(LogTemp, Warning, TEXT("Session Server Shutdown after %f without player joining!"), WaitPlayerJoinTimeoutDuration);
	TerminateSessionServer();
}

void UChrisGameInstance::DestroyBeforeJoinCompleted(FName SessionName, bool bWasSuccessful)
{
	if (IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr())
	{
		SessionPtr->OnDestroySessionCompleteDelegates.RemoveAll(this);
	}

	JoinedSessionName = NAME_None;

	UE_LOG(LogTemp, Warning, TEXT("[Session] Destroy before join completed: %d"), bWasSuccessful ? 1 : 0);

	const FString Id = PendingJoinSessionId;
	PendingJoinSessionId.Empty();

	// GetNamedSession is null now, so this takes the normal path
	if (!Id.IsEmpty())
	{
		JoinSessionWithId(Id);
	}
}


void UChrisGameInstance::LoadLevelAndListen(TSoftObjectPtr<UWorld> Level)
{
	// Convert the soft object path into a mappable package name 
	const FName LevelURL = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));

	// ServerTravel with "?listen" so the server accepts client connections on the new map
	if (LevelURL != "")
	{
		FString TravelString = FString::Printf(TEXT("%s?listen?port=%d"), *LevelURL.ToString(), SessionServerPort);
		UE_LOG(LogTemp, Warning, TEXT("Server travelling to: %s"), *(TravelString));
		GetWorld()->ServerTravel(TravelString);
	}
}

void UChrisGameInstance::ReportSessionStatusToCoordinator(const FString& Status)
{
	const FString SearchId = UChrisNetStatics::GetSessionSearchIdString();
	if (SearchId.IsEmpty()) { return; }

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(UChrisNetStatics::GetCoordinatorURLString() + TEXT("/SessionStatus"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	const FString Body = FString::Printf(
		TEXT("{\"%s\":\"%s\",\"STATUS\":\"%s\"}"),
		*UChrisNetStatics::GetSessionSearchIdKey().ToString(),
		*SearchId,
		*Status);

	Request->SetContentAsString(Body);
	Request->ProcessRequest();
}

void UChrisGameInstance::SendHeartbeat()
{
	ReportSessionStatusToCoordinator(TEXT("alive"));
}

void UChrisGameInstance::FetchJoinableSessions()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(UChrisNetStatics::GetCoordinatorURLString() + TEXT("/Sessions"));
	Request->SetVerb(TEXT("GET"));
	Request->OnProcessRequestComplete().BindUObject(this, &UChrisGameInstance::JoinableSessionsFetched);
	Request->ProcessRequest();
}

void UChrisGameInstance::JoinableSessionsFetched(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	if (!bSuccess || !Response.IsValid()) { return; }

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid()) { return; }

	const TArray<TSharedPtr<FJsonValue>>* Ids = nullptr;
	if (!Json->TryGetArrayField(TEXT("SESSIONS"), Ids)) { return; }

	JoinableSessionIds.Empty();
	for (const TSharedPtr<FJsonValue>& Value : *Ids)
	{
		JoinableSessionIds.Add(Value->AsString());
	}
}
void UChrisGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Warning, TEXT("[Session] Network failure: %s"), *ErrorString);

	GetWorld()->GetTimerManager().ClearTimer(JoinSessionTimeoutHandle);
	StopAllSessionFindings();

	// Without this the client stays registered in a session it isn't connected
	// to, and the next join attempt fails for no obvious reason
	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (SessionPtr)
	{
		SessionPtr->DestroySession(JoinedSessionName);
	}

	bReturnToMultiplayerPage = true;
	PendingMenuMessage = NSLOCTEXT("MainMenu", "JoinFailed", "COULD NOT JOIN THIS SESSION");
	OnTravelFailedWithReason.Broadcast(ErrorString);
}

void UChrisGameInstance::StartPracticeArena()
{
	bPracticeMode = true;

	const FString LevelName = FPackageName::ObjectPathToPackageName(Lvl_ThirdPerson.ToString());

	// Pass as a URL option too: PIE can rebuild the game instance across a map change, which loses the member flag entirely
	UGameplayStatics::OpenLevel(this, FName(*LevelName), true, TEXT("practice=1"));

	UE_LOG(LogTemp, Warning, TEXT("[Practice] Flag SET before travel"));
}
