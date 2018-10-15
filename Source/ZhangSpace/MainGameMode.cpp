// Copyright Team Monkey Business 2018.

#include "MainGameMode.h"
#include "MainPlayerController.h"
#include "MainGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Character.h"
#include "ConstructorHelpers.h"

//#include "Runtime/Engine/Classes/GameFramework/GameSession.h"

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

//Called every frame
void AMainGameMode::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	//TODO, start game after players have joined, keep track of all players

	if (_playerCount > 0)
	{
		_connectedPlayersCheckTimer += DeltaTime;

		if (_connectedPlayersCheckTimer >= 5.0f)
		{
			if (NumPlayers == 0)
			{
				GIsRequestingExit = true;
				return;
			}

			_connectedPlayersCheckTimer = 0.0f;
		}
	}

	/*
	//GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, "Rotation: " + FString::FromInt (NumPlayers));
	UE_LOG (LogTemp, Log, TEXT ("There are currently %d players connected."), NumPlayers);

	if (_shutdownTimer < 5.0f)
	{
		_shutdownTimer += DeltaTime;

		if (_shutdownTimer >= 5.0f)
		{
			//GameSession->KickPlayer (
			//GIsRequestingExit = true;
		}
	}*/
}

AActor* AMainGameMode::ChoosePlayerStart_Implementation (AController* Player)
{
	//TODO, fix this and do stuff properly

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
