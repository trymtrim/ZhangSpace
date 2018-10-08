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
	TArray <AActor*> playerStarts;
	UGameplayStatics::GetAllActorsOfClass (GetWorld (), APlayerStart::StaticClass (), playerStarts);

	if (_playerCount == _maxPlayers)
	{
		if (_currentPlayerStartIndex == playerStarts.Num () - 1)
			_currentPlayerStartIndex = 0;
		else
			_currentPlayerStartIndex++;

		return playerStarts [_currentPlayerStartIndex];
	}

	if (_playerCount < _maxPlayers)
		_playerCount++;

	//GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, Player->GetName ());
	//GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, playerStarts [_playerCount - 1]->GetName ());
	
	return playerStarts [_playerCount - 1];
}
