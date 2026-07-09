// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameInstance.h"
#include "ChrisGameInstance.h"
#include "Network/ChrisNetStatics.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "HttpModule.h"

// Only the server (dedicated or listen) should initiate a map travel
void UChrisGameInstance::StartMatch()
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		LoadLevelAndListen(Lvl_ThirdPerson);
	}
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

	if(!Request->ProcessRequest())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Creation Request failed right away!"));
	}
}

void UChrisGameInstance::CancelSessionCreation()
{
	UE_LOG(LogTemp, Warning, TEXT("Cancelling session creation"));
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

void UChrisGameInstance::StopFindingCreatedSession()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop finding Created Session!"));
}


void UChrisGameInstance::StopGlobalSessionSearch()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop global session search!"));
}


void UChrisGameInstance::FindCreatedSession(FGuid SessionSearchId)
{
	UE_LOG(LogTemp, Warning, TEXT("Trying to find created session!"));
}

void UChrisGameInstance::FindCreatedSessionTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("Created session timeout reached!"));
	StopFindingCreatedSession();
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

void UChrisGameInstance::EndSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	FGenericPlatformMisc::RequestExit(false);
}

void UChrisGameInstance::WaitPlayerJoinTimeoutReached()
{
	UE_LOG(LogTemp, Warning, TEXT("Session Server Shutdown after %f without player joining!"), WaitPlayerJoinTimeoutDuration);
	TerminateSessionServer();
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
