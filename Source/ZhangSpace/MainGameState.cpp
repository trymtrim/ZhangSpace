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
}

void AMainGameState::DamagePlayersOutsideOfCircle ()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
		{
			if (FVector::Distance (_shrinkingCircle->GetActorLocation (), playerController->GetCharacter ()->GetActorLocation ()) > _shrinkingCircle->GetActorScale ().X * 50.0f)
			{
				float damage = 3.0f;

				UGameplayStatics::ApplyDamage (playerController->GetCharacter (), damage, nullptr, _shrinkingCircle, nullptr);
			}
		}
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
}

void AMainGameState::StartGame ()
{
	_gameStarted = true;
}

void AMainGameState::AddPlayerKill (AMainPlayerController* playerController)
{
	int playerIndex = _playerIndexes [playerController];
	playerKills [playerIndex]++;
}

void AMainGameState::UpdatePlayerLives (AMainPlayerController* playerController, int lives)
{
	int playerIndex = _playerIndexes [playerController];
	playerLives [playerIndex] = lives;
}

void AMainGameState::GetLifetimeReplicatedProps (TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps (OutLifetimeProps);

	DOREPLIFETIME (AMainGameState, playerNames);
	DOREPLIFETIME (AMainGameState, playerKills);
	DOREPLIFETIME (AMainGameState, playerLives);
}
