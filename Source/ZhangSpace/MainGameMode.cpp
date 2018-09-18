// Copyright Team Monkey Business 2018.

#include "MainGameMode.h"
#include "MainPlayerController.h"
#include "MainGameState.h"

AMainGameMode::AMainGameMode ()
{
	//Set default pawn class
	static ConstructorHelpers::FClassFinder <APawn> PlayerPawnClass (TEXT ("/Game/Blueprints/MainCharacterController"));

	if (PlayerPawnClass.Class != NULL)
		DefaultPawnClass = PlayerPawnClass.Class;

	//Set default player controller class
	PlayerControllerClass = AMainPlayerController::StaticClass ();

	//Set default game state class
	GameStateClass = AMainGameState::StaticClass ();
}

/*AActor* AMainGameMode::ChoosePlayerStart_Implementation (AController* Player)
{
	GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, Player->GetName ());
	return nullptr;
}*/
