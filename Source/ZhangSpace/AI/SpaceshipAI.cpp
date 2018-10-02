// Copyright Team Monkey Business 2018.

#include "SpaceshipAI.h"
#include "Engine/World.h"
#include "MainPlayerController.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"

//Sets default values
ASpaceshipAI::ASpaceshipAI ()
{
	//Set this actor to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void ASpaceshipAI::BeginPlay ()
{
	Super::BeginPlay ();
}

//Called every frame
void ASpaceshipAI::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
		ServerUpdate (DeltaTime);
}

void ASpaceshipAI::ServerUpdate (float deltaTime)
{
	//Update behaviour based on current state
	switch (_state)
	{
	case PATROL:
		UpdatePatrolState (deltaTime);
		break;
	case ATTACK:
		UpdateAttackState (deltaTime);
		break;
	case DEAD:

		break;
	}
}

void ASpaceshipAI::UpdatePatrolState (float deltaTime)
{
	//If a player has aggro, change to attack state
	if (CheckForAggro ())
		_state = ATTACK;
}

void ASpaceshipAI::UpdateAttackState (float deltaTime)
{
	//If target doesn't exist, change to patrol state
	if (_target == nullptr)
	{
		_state = PATROL;
		return;
	}

	//Update attack cooldown
	if (_currentAttackCooldown > 0.0f)
		_currentAttackCooldown -= deltaTime;

	//Check if there is no attack cooldown
	if (_currentAttackCooldown <= 0.0f)
	{
		//If the player target is in sight, shoot
		FRotator lookAt = UKismetMathLibrary::FindLookAtRotation (GetActorLocation (), _target->GetActorLocation ());
		
		if (lookAt.Yaw > 170 && lookAt.Yaw < 190 && lookAt.Pitch > -15 && lookAt.Pitch < 15 && lookAt.Roll > -15 && lookAt.Roll < 15)
			Shoot ();
	}

	//If target is out of lose-aggro range, change to patrol state
	if (FVector::Distance (GetActorLocation (), _target->GetActorLocation ()) > _loseAggroRange)
	{
		_target = nullptr;
		_state = PATROL;
	}
}

bool ASpaceshipAI::CheckForAggro ()
{
	//Get a list of all players
	TArray <ACharacter*> players;

	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
			players.Add (playerController->GetCharacter ());
	}

	//If a player is within aggro range, set that player as target
	for (int i = 0; i < players.Num (); i++)
	{
		ACharacter* player = players [i];

		if (FVector::Distance (GetActorLocation (), player->GetActorLocation ()) <= _aggroRange)
		{
			_target = player;
			return true;
		}
	}

	return false;
}

void ASpaceshipAI::Shoot ()
{
	//Declare spawn parameters
	FActorSpawnParameters spawnParams;
	FVector spawnPosition = GetActorLocation () + GetActorForwardVector () * 350.0f;
	FRotator spawnRotation = GetActorRotation ();

	//Spawn projectile
	AProjectile* projectile = GetWorld ()->SpawnActor <AProjectile> (_projectileBP, spawnPosition, spawnRotation, spawnParams);

	//Set projectile damage
	if (projectile->IsValidLowLevel () && projectile != nullptr)
		projectile->SetDamage (10.0f);

	//Reset attack cooldown
	_currentAttackCooldown = _maxAttackCooldown;
}
