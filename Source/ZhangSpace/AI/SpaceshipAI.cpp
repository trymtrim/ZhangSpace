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

	if (GetWorld ()->IsServer ())
	{
		//Get references to the gun positions
		TArray <UArrowComponent*> arrowComps;
		GetComponents <UArrowComponent> (arrowComps);

		for (int i = 0; i < arrowComps.Num (); i++)
		{
			if (arrowComps [i]->GetName () == "GunPosition1")
				gunPositionOne = arrowComps [i];
			else if (arrowComps [i]->GetName () == "GunPosition2")
				gunPositionTwo = arrowComps [i];
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
	//AddActorLocalRotation ((_target->GetActorLocation () - GetActorLocation ()).ToOrientationRotator ());
	SetActorRotation (FMath::Lerp (GetActorRotation (), (_target->GetActorLocation () - GetActorLocation ()).ToOrientationRotator (), deltaTime * 2.5f));
	//Update attack cooldown
	if (_currentAttackCooldown > 0.0f)
		_currentAttackCooldown -= deltaTime;

	//Check if there is no attack cooldown
	if (_currentAttackCooldown <= 0.0f)
	{
		//If the player target is in sight, shoot
		//if (IsAttackableInScope ())
			Shoot ();

		/*FRotator lookAt = UKismetMathLibrary::FindLookAtRotation (GetActorLocation (), _target->GetActorLocation ());
		
		if (lookAt.Yaw > 170 && lookAt.Yaw < 190 && lookAt.Pitch > -15 && lookAt.Pitch < 15 && lookAt.Roll > -15 && lookAt.Roll < 15)
			Shoot ();*/
	}
	
	//If target is out of lose-aggro range, change to patrol state
	if (FVector::Distance (GetActorLocation (), _target->GetActorLocation ()) > _loseAggroRange || _target->GetIsDead ())
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
	//Line trace from camera to check if there is something in the crosshair's sight
	FCollisionQueryParams traceParams = FCollisionQueryParams (FName (TEXT ("RV_Trace")), true, this);
	traceParams.bTraceComplex = true;
	traceParams.bReturnPhysicalMaterial = false;

	FHitResult hit (ForceInit);

	//Declare start and end position of the line trace based on camera position and rotation
	FVector start = GetActorLocation ();
	FVector end = GetActorLocation () + (GetActorForwardVector () * 400000.0f);

	//Declare spawn parameters
	FActorSpawnParameters spawnParams;
	FVector spawnPosition;
	FRotator spawnRotation = GetActorRotation ();

	//Spawn projectile at assigned gun position
	if (_gunPositionSwitch)
		spawnPosition = gunPositionOne->GetComponentLocation ();
	else
		spawnPosition = gunPositionTwo->GetComponentLocation ();

	_gunPositionSwitch = !_gunPositionSwitch;

	//Check if line trace hits anything
	if (GetWorld ()->LineTraceSingleByChannel (hit, start, end, ECC_Visibility, traceParams))
	{
		//If line trace hits a projectile, spawn bullet with rotation towards the end of the line trace
		if (hit.GetActor ()->ActorHasTag ("Projectile"))
			spawnRotation = (end - spawnPosition).Rotation ();
		else //Otherwise, spawn bullet with rotation towards what it hits
			spawnRotation = (hit.ImpactPoint - spawnPosition).Rotation ();
	}
	else //If line trace doesn't hit anything, spawn bullet with rotation towards the end of the line trace
		spawnRotation = (end - GetActorLocation ()).Rotation ();

	//Spawn projectile
	AProjectile* projectile = GetWorld ()->SpawnActor <AProjectile> (_projectileBP, spawnPosition, spawnRotation, spawnParams);
	projectile->SetOwner (this);

	//Set projectile damage
	if (projectile->IsValidLowLevel () && projectile != nullptr)
		projectile->SetDamage (FMath::RandRange (5, 15));

	//Reset attack cooldown
	_currentAttackCooldown = _maxAttackCooldown;
}

void ASpaceshipAI::Die ()
{
	DieBP ();

	Destroy ();
}

float ASpaceshipAI::TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	_health -= Damage;

	//If health is below zero, die
	if (_health <= 0)
		Die ();

	return Super::TakeDamage (Damage, DamageEvent, EventInstigator, DamageCauser);
}
