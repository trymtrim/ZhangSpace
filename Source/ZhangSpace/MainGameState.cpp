// Copyright Team Monkey Business 2018.

#include "MainGameState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "MainPlayerController.h"
#include "GameFramework/Character.h"
#include "UnrealNetwork.h"
#include "MainGameMode.h"

AMainGameState::AMainGameState ()
{
	//Set this character to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void AMainGameState::BeginPlay ()
{
	Super::BeginPlay ();

	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
	{
		//Find available spells from scene
		TArray <AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass (GetWorld (), AShrinkingCircle::StaticClass (), actors);
		_shrinkingCircle = Cast <AShrinkingCircle> (actors [0]);
	}
}

//Called every frame
void AMainGameState::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
		ServerUpdate (DeltaTime);
}

void AMainGameState::ServerUpdate (float deltaTime)
{
	if (!_gameStarted)
		return;

	//Update damage timer
	_damageTimer += deltaTime;

	//Damage everyone player who is outside of the circle based on damage interval
	if (_damageTimer >= _damageInterval)
	{
		DamagePlayersOutsideOfCircle ();
		_damageTimer = 0.0f;
	}

	if (_gameStarted && !_gameFinished)
		gameTimer += deltaTime;
}

void AMainGameState::DamagePlayersOutsideOfCircle ()
{
	if (!_gameStarted || flyingIn || _gameFinished)
		return;

	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
		{
			if (FVector::Distance (_shrinkingCircle->GetActorLocation (), playerController->GetCharacter ()->GetActorLocation ()) > _shrinkingCircle->GetActorScale ().X * 50.0f)
			{
				if (!playerController->flyingIn)
				{
					float damage = 3.0f;

					UGameplayStatics::ApplyDamage (playerController->GetCharacter (), damage, nullptr, _shrinkingCircle, nullptr);
				}
			}
			else if (playerController->flyingIn)
			{
				playerController->flyingIn = false;

				AMainCharacterController* characterController = Cast <AMainCharacterController> (playerController->GetCharacter ());
				characterController->flyingIn = false;

				characterController->FinishFlyingInBP ();
			}
		}
	}
}

void AMainGameState::UpdateFeedText (FString feedText)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
			Cast <AMainCharacterController> (playerController->GetCharacter ())->UpdateFeedText (feedText);
	}
}

void AMainGameState::RegisterPlayer (AMainPlayerController* playerController, FString playerName, int targetPlayerCount)
{
	Cast <AMainGameMode> (GetWorld ()->GetAuthGameMode ())->RegisterPlayer (playerController, targetPlayerCount);

	_playerIndexes.Add (playerController, _players.Num ());
	_players.Add (playerController);
	
	playerNames.Add (playerName);
	playerKills.Add (0);
	playerLives.Add (3);
	playerLevels.Add (1);
}

void AMainGameState::StartGame ()
{
	_gameStarted = true;

	if (!_hasShownStartFeedText)
	{
		FTimerHandle startFeedTextTimerHandle;
		GetWorld ()->GetTimerManager ().SetTimer (startFeedTextTimerHandle, this, &AMainGameState::ShowStartFeedText, 2.0f, false);

		_hasShownStartFeedText = true;
	}

	_shrinkingCircle->gameStarted = true;
}

void AMainGameState::ShowStartFeedText ()
{
	UpdateFeedText ("Move sideways, up or down to choose your\ndestination in the play area. Full control of your\nship will be granted when you enter the force field.");
}

void AMainGameState::AddPlayerKill (AMainPlayerController* playerController)
{
	int playerIndex = _playerIndexes [playerController];
	playerKills [playerIndex]++;
}

void AMainGameState::AddPlayerLevel (AMainPlayerController* playerController)
{
	int playerIndex = _playerIndexes [playerController];
	playerLevels [playerIndex]++;
}

void AMainGameState::UpdatePlayerLives (AMainPlayerController* playerController, int lives)
{
	int playerIndex = _playerIndexes [playerController];
	playerLives [playerIndex] = lives;

	//Check for victory
	int deadPlayers = 0;
	FString winnerName = "";

	if (playerLives.Num () == 1)
	{
		if (playerLives [0] == 0)
			FinishGame ("Nobody");
	}
	else
	{
		for (int i = 0; i < playerLives.Num (); i++)
		{
			if (playerLives [i] == 0)
				deadPlayers++;
			else
				winnerName = playerNames [i];
		}

		if (deadPlayers == playerLives.Num () - 1)
			FinishGame (winnerName);
	}
}

void AMainGameState::FinishGame (FString winnerName)
{
	for (int i = 0; i < playerLives.Num (); i++)
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
		{
			AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

			if (playerController)
				Cast <AMainCharacterController> (playerController->GetCharacter ())->FinishGame (winnerName);
		}
	}

	_gameFinished = true;
}

void AMainGameState::SpawnFracturedSpaceship (TSubclassOf <AActor> fracturedSpaceshipBP, FTransform transform)
{
	//Declare spawn parameters
	FActorSpawnParameters spawnParams;
	FVector spawnPosition = transform.GetLocation ();
	FRotator spawnRotation = transform.GetRotation ().Rotator ();

	//Spawn fractured spaceship
	GetWorld ()->SpawnActor <AActor> (fracturedSpaceshipBP, spawnPosition, spawnRotation, spawnParams);
}

void AMainGameState::GetLifetimeReplicatedProps (TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps (OutLifetimeProps);

	DOREPLIFETIME (AMainGameState, playerNames);
	DOREPLIFETIME (AMainGameState, playerKills);
	DOREPLIFETIME (AMainGameState, playerLives);
	DOREPLIFETIME (AMainGameState, playerLevels);
}
