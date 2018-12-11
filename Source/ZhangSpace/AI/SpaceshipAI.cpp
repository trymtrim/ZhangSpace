// Copyright Team Monkey Business 2018.

#include "SpaceshipAI.h"
#include "Engine/World.h"
#include "MainPlayerController.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "MainGameState.h"

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

	if (GetWorld ()->IsServer ())
	{
		//Get a reference to the game state
		while (_gameState == nullptr)
		{
			if (GetWorld ()->GetGameState () != nullptr)
				_gameState = Cast <AMainGameState> (GetWorld ()->GetGameState ());
		}
	}
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
	}

	//Update "attack player out of range" timer
	if (_shootOutOfRangeTimer > 0.0f)
		_shootOutOfRangeTimer -= deltaTime;
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

	//Rotate towards player target
	SetRotationBP ((_target->GetActorLocation () - GetActorLocation ()).ToOrientationRotator (), _target->GetActorLocation ());

	//Update attack cooldown
	if (_currentAttackCooldown > 0.0f)
		_currentAttackCooldown -= deltaTime;

	//Check if there is no attack cooldown
	if (_currentAttackCooldown <= 0.0f)
	{
		//If the player target is in sight, shoot
		//if (IsAttackableInScope ())

		if (!_target->GetIsDead ())
			Shoot ();
		else
		{
			_target = nullptr;
			_state = PATROL;

			return;
		}
	}
	
	//If target is out of lose-aggro range, change to patrol state
	if (FVector::Distance (GetActorLocation (), _target->GetActorLocation ()) > _loseAggroRange && _shootOutOfRangeTimer <= 0.0f || _target->GetIsDead ())
	{
		_target = nullptr;
		_state = PATROL;
	}
}

bool ASpaceshipAI::CheckForAggro ()
{
	//Get a list of all players
	TArray <AMainCharacterController*> players;

	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
			players.Add (Cast <AMainCharacterController> (playerController->GetCharacter ()));
	}

	//If a player is within aggro range, set that player as target
	for (int i = 0; i < players.Num (); i++)
	{
		AMainCharacterController* player = players [i];

		if (FVector::Distance (GetActorLocation (), player->GetActorLocation ()) <= _aggroRange && !player->GetIsDead ())
		{
			_target = player;
			return true;
		}
	}

	return false;
}

bool ASpaceshipAI::IsAttackableInScope ()
{
	//Line trace from camera to check if there is something in the crosshair's sight
	FCollisionQueryParams traceParams = FCollisionQueryParams (FName (TEXT ("RV_Trace")), true, this);
	traceParams.bTraceComplex = true;
	traceParams.bReturnPhysicalMaterial = false;

	FHitResult hit (ForceInit);

	//Declare start and end position of the line trace based on camera position and rotation
	FVector start = GetActorLocation ();
	FVector end = GetActorLocation () + (GetActorForwardVector () * 400000.0f);

	if (GetWorld ()->LineTraceSingleByChannel (hit, start, end, ECC_Visibility, traceParams))
	{
		//If line trace hits a projectile, spawn bullet with rotation towards the end of the line trace
		if (hit.GetActor ()->ActorHasTag ("Player"))
			return true;
	}

	return false;
}

void ASpaceshipAI::Shoot ()
{
	if (_disarmed)
		return;

	ShootBP (_gunPositionSwitch, FMath::RandRange (5, 15), this, _target);

	_gunPositionSwitch = !_gunPositionSwitch;

	//projectile->SetOwner (this);

	//Reset attack cooldown
	_currentAttackCooldown = _maxAttackCooldown;
}

void ASpaceshipAI::DealBeamDamage (int damage, AMainCharacterController* player)
{
	_target = player;

	_state = ATTACK;
	_shootOutOfRangeTimer = 5.0f;

	_beamDamage += damage;

	if (_beamDamage > 1.0f)
	{
		_beamDamage -= 1.0f;

		_health -= 1;

		player->UpdatePlayerHitText (0, 1);

		if (_health <= 0)
			Die ();
	}
}

void ASpaceshipAI::Die ()
{
	DieBP ();

	Destroy ();
}

void ASpaceshipAI::ProtectResource (AMainCharacterController* playerTarget)
{
	if (_state != PATROL)
		return;

	_target = playerTarget;
	_state = ATTACK;
	_shootOutOfRangeTimer = 5.0f;
}

void ASpaceshipAI::Disarm ()
{
	_disarmed = true;

	FTimerHandle disarmTimerHandle;
	GetWorld ()->GetTimerManager ().SetTimer (disarmTimerHandle, this, &ASpaceshipAI::CancelDisarm, 3.0f, false);
}

void ASpaceshipAI::CancelDisarm ()
{
	_disarmed = false;
}

float ASpaceshipAI::TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	_health -= Damage;

	if (DamageCauser->GetOwner ()->ActorHasTag ("Player"))
	{
		_target = Cast <AMainCharacterController> (DamageCauser->GetOwner ());

		_state = ATTACK;
		_shootOutOfRangeTimer = 5.0f;

		_target->UpdatePlayerHitText (0, Damage);
	}

	//If health is below zero, die
	if (_health <= 0)
		Die ();

	return Super::TakeDamage (Damage, DamageEvent, EventInstigator, DamageCauser);
}
