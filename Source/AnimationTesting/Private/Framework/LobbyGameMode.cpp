// Christopher Naglik All Rights Reserved


#include "Framework/LobbyGameMode.h"
#include "Network/ChrisGameSession.h"

ALobbyGameMode::ALobbyGameMode()
{
	bUseSeamlessTravel = true;
	GameSessionClass = AChrisGameSession::StaticClass();
}
