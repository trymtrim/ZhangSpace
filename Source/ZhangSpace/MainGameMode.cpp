// Copyright Team Monkey Business 2018.

#include "MainGameMode.h"
#include "MainPlayerController.h"
#include "MainGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Character.h"

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

AActor* AMainGameMode::ChoosePlayerStart_Implementation (AController* Player)
{
	if (_playerCount < _maxPlayers)
		_playerCount++;

	TArray <AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass (GetWorld (), APlayerStart::StaticClass (), actors);

	GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, Player->GetName ());
	GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, actors [_playerCount - 1]->GetName ());
	
	return actors [_playerCount - 1];
}
