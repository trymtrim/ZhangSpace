// Copyright Team Monkey Business 2018.

#include "MainCharacterController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MainGameState.h"
#include "Teleporter.h"
#include "AI/SpaceshipAI.h"

#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Engine.h"

AMainCharacterController::AMainCharacterController ()
{
 	//Set this character to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void AMainCharacterController::BeginPlay ()
{
	Super::BeginPlay ();

	InitializeAbilityCooldowns ();

	//Change mesh on client-side
	if (!GetWorld ()->IsServer () && IsLocallyControlled ())
	{
		//Change to cockpit mesh
		ChangeMesh (_cockpitMesh);

		//Get camera component
		TArray <UCameraComponent*> cameraComps;
		GetComponents <UCameraComponent> (cameraComps);
		_cameraComponent = cameraComps [0];

		DisableMouseCursor ();

		//FPS
		FTimerHandle FPSTimerHandle;
		GetWorld ()->GetTimerManager ().SetTimer (FPSTimerHandle, this, &AMainCharacterController::UpdateFPS, 1.0f, true);

		//Add shield ability
		AddAbility (0);
	}

	if (GetWorld ()->IsServer ())
	{
		//Get a reference to the game state
		while (_gameState == nullptr)
		{
			if (GetWorld ()->GetGameState () != nullptr)
				_gameState = Cast <AMainGameState> (GetWorld ()->GetGameState ());
		}

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

void AMainCharacterController::UpdateFPS ()
{
	FPSText = FString::FromInt (FPS);
	FPS = 0;
}

//Called every frame
void AMainCharacterController::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	//Update cooldowns server side
	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
	{
		UpdateStats (DeltaTime);
		UpdateShootingCooldown (DeltaTime);

		if (_dead)
			UpdateDeadState (DeltaTime);

		if (resetPlayerHitTimer > 0.0f)
			resetPlayerHitTimer -= DeltaTime;

	}

	//Update stats for the client's UI
	if (!GetWorld ()->IsServer () && IsLocallyControlled ())
	{
		UpdateStatsUI ();

		if (_shooting)
			UpdateShooting ();

		if (_channelingBeam)
		{
			FVector cameraPosition = _cameraComponent->GetComponentLocation ();
			FVector cameraForward = _cameraComponent->GetForwardVector ();

			ChannelHyperBeam (cameraPosition, cameraForward);
		}

		FPS++;
	}
}

void AMainCharacterController::InitializeAbilityCooldowns ()
{
	_abilityMaxCooldowns.Add (0, 60.0f); //Shield
	_abilityMaxCooldowns.Add (1, 10.0f); //Attack 1
	_abilityMaxCooldowns.Add (2, 10.0f); //Attack 2
	_abilityMaxCooldowns.Add (3, 10.0f); //Attack 3
	_abilityMaxCooldowns.Add (4, 30.0f); //Cloak
	_abilityMaxCooldowns.Add (5, 10.0f); //Defense 2
	_abilityMaxCooldowns.Add (6, 10.0f); //Defense 3
	_abilityMaxCooldowns.Add (7, 15.0f); //Teleport
	_abilityMaxCooldowns.Add (8, 10.0f); //Mobility 2
	_abilityMaxCooldowns.Add (9, 10.0f); //Mobility 3

	//Add shield ability to hotkey bar
	_hotkeyBarAbilities.Add (0);
}

void AMainCharacterController::ChangeMesh (UStaticMesh* mesh)
{
	TArray <UStaticMeshComponent*> meshComps;
	GetComponents <UStaticMeshComponent> (meshComps);
	UStaticMeshComponent* meshComponent;

	if (GetWorld ()->WorldType == EWorldType::Game)
		meshComponent = meshComps [0];
	else
		meshComponent = meshComps [1];

	meshComponent->SetStaticMesh (mesh);
}

void AMainCharacterController::UpdateFeedText_Implementation (const FString& feedText)
{
	AddFeedTextBP (feedText);
}

void AMainCharacterController::Die ()
{
	//Update lives and health
	_lives--;
	_currentHealth = 0;
	_dead = true;

	cloakActive = false;

	if (_channelingBeam)
		CancelHyperBeam ();

	if (_isBoosting)
		CancelBoost ();

	_isUsingTrapShot = false;

	DieBP ();

	//Stop movement
	GetCharacterMovement ()->StopMovementImmediately ();

	//Update lives in game state
	AMainPlayerController* playerController = Cast <AMainPlayerController> (GetController ());
	_gameState->UpdatePlayerLives (playerController, _lives);

	//If the player has no lives left, call game over
	if (_lives == 0)
		GameOver ();
}

void AMainCharacterController::UpdateDeadState (float deltaTime)
{
	if (_lives == 0)
		return;

	//Update respawn timer
	_currentDeadTimer += deltaTime;

	//If respawn timer is finished, respawn
	if (_currentDeadTimer >= _maxDeadTimer)
	{
		_currentDeadTimer = 0.0f;
		Respawn ();
	}
}

void AMainCharacterController::Respawn ()
{
	//Reset attributes
	_power = _maxPower;
	_currentHealth = _maxHealth;

	//Reset ability cooldowns
	for (int i = 0; i < _abilities.Num (); i++)
		_abilityCooldowns [i] = 0.0f;

	//Set new location to a random player start
	/*TArray <AActor*> playerStarts;
	UGameplayStatics::GetAllActorsOfClass (GetWorld (), APlayerStart::StaticClass (), playerStarts);

	int randomIndex = FMath::RandRange (0, playerStarts.Num () - 1);
	SetActorLocation (playerStarts [randomIndex]->GetActorLocation ());*/

	int randomIndex = FMath::RandRange (0, 4);
	
	switch (randomIndex)
	{
	case 0:
		SetActorLocation (FVector (-10000.0f, -4500.0f, 12400.0f));
		break;
	case 1:
		SetActorLocation (FVector (15000.0f, -10900.0f, 1700.0f));
		break;
	case 2:
		SetActorLocation (FVector (-2700.0f, 16000.0f, -6600.0f));
		break;
	case 3:
		SetActorLocation (FVector (-1250.0f, 1600.0f, -3300.0f));
		break;
	}

	RespawnBP ();

	_dead = false;
}

void AMainCharacterController::FinishGame (FString winnerName)
{
	_gameFinished = true;

	cloakActive = false;

	if (_channelingBeam)
		CancelHyperBeam ();

	if (_isBoosting)
		CancelBoost ();

	_isUsingTrapShot = false;

	//Stop movement
	GetCharacterMovement ()->StopMovementImmediately ();

	FinishGameBP (winnerName);
}

void AMainCharacterController::GameOver ()
{
	_gameOver = true;

	//Do stuff
}

bool AMainCharacterController::GetGameOver ()
{
	return _gameOver;
}

void AMainCharacterController::AddExperience (int experience)
{
	if (systemLevel == 15)
		return;

	//Add the gained experience to the player
	_experience += experience;

	//If experience is higher than needed level-up experience, level up
	if (_experience >= _experienceToNextLevel)
	{
		_level++;

		_experience = _experience - _experienceToNextLevel;

		//Set new experience cap
		_experienceToNextLevel = 80 + (_level * 20);

		AddAvailableStats ();

		systemLevel++;

		UpdatePlayerHitText (-2, 0);

		if (systemLevel == 5 || systemLevel == 10 || systemLevel == 15)
			_gameState->UpdateFeedText (_gameState->playerNames [playerID - 1] + " has reached system level " + FString::FromInt (systemLevel) + ".");

		if (systemLevel == 15)
		{
			_experience = _experienceToNextLevel;
			_availableStats--;
		}
	}
}

void AMainCharacterController::AddAbility (int abilityIndex)
{
	//If the ability is a passive, enable the respective bool client-side
	if (abilityIndex == 5 || abilityIndex == 6)
	{
		switch (abilityIndex)
		{
		case 5: //Shield Ram
			shieldRam = true;
			break;
		case 6: //Shield Reflect
			_shieldReflect = true;
			break;
		}
	}

	ServerAddAbility (abilityIndex);
}

void AMainCharacterController::ServerAddAbility_Implementation (int abilityIndex)
{
	//If the ability is a passive, enable the respective bool, otherwise add it the the ability list
	if (abilityIndex == 5 || abilityIndex == 6)
	{
		switch (abilityIndex)
		{
		case 5: //Shield Ram
			shieldRam = true;
			break;
		case 6: //Shield Reflect
			_shieldReflect = true;
			break;
		}
	}
	else
	{
		//Add ability to the list of abilities
		_abilities.Add (abilityIndex);
		//Add ability to the list of ability cooldowns
		_abilityCooldowns.Add (0.0f);
	}

	if (_attackUpgradesAvailable != 0 || _defenseUpgradesAvailable != 0 || _mobilityUpgradesAvailable != 0)
	{
		if (abilityIndex <= 3)
			_attackUpgradesAvailable--;
		else if (abilityIndex > 3 && abilityIndex <= 6)
			_defenseUpgradesAvailable--;
		else if (abilityIndex > 6 && abilityIndex <= 9)
			_mobilityUpgradesAvailable--;
	}
}

bool AMainCharacterController::ServerAddAbility_Validate (int abilityIndex)
{
	return true;
}

void AMainCharacterController::AddAvailableStats ()
{
	//Add two stat points to the player's available stats
	_availableStats += 2;
}

void AMainCharacterController::AddStat_Implementation (int statIndex)
{
	if (_availableStats == 0)
		return;

	switch (statIndex)
	{
	case 1: //Attack
		if (_attackPower == _maxStatPower)
			return;

		_attackPower++;

		//Decrease shooting cooldown
		_maxShootingCooldown -= _maxShootingCooldown * 0.1f;

		//GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, FString::SanitizeFloat (_maxShootingCooldown));

		if (_attackPower == 3 || _attackPower == 6 || _attackPower == 10)
			_attackUpgradesAvailable++;
		break;
	case 2: //Defense
		if (_defensePower == _maxStatPower)
			return;

		_defensePower++;

		//Decrease shield cooldown
		_abilityMaxCooldowns [0] -= _abilityMaxCooldowns [0] * 0.1f;
		ClientChangeShieldCooldown (_abilityMaxCooldowns [0]);

		if (_defensePower == 3 || _defensePower == 6 || _defensePower == 10)
			_defenseUpgradesAvailable++;
		break;
	case 3: //Mobility
		if (_mobilityPower == _maxStatPower)
			return;

		_mobilityPower++;

		if (_mobilityPower == 3 || _mobilityPower == 6 || _mobilityPower == 10)
			_mobilityUpgradesAvailable++;
		break;
	}

	_availableStats--;
}

bool AMainCharacterController::AddStat_Validate (int statIndex)
{
	return true;
}

void AMainCharacterController::ClientChangeShieldCooldown_Implementation (int cooldown)
{
	_abilityMaxCooldowns [0] = cooldown;
}

void AMainCharacterController::UseAbilityInput (int abilityIndex)
{
	if (!gameStarted || _dead || _inSettingsMenu || _hotkeyBarAbilities.Num () < abilityIndex)
		return;

	FVector cameraPosition = _cameraComponent->GetComponentLocation ();

	int actualAbilityIndex = _hotkeyBarAbilities [abilityIndex -1];
	UseAbility (actualAbilityIndex, cameraPosition);
}

void AMainCharacterController::UseAbility_Implementation (int abilityIndex, FVector cameraPosition)
{
	if (disarmed || !_abilities.Contains (abilityIndex) || !gameStarted || flyingIn || _dead || _gameFinished)
		return;

	//Get ability index, if ability is on cooldown, return
	int actualAbilityIndex = 0;

	for (int i = 0; i < _abilities.Num (); i++)
	{
		if (_abilities [i] == abilityIndex)
		{
			if (_abilityCooldowns [i] > 0.0f)
				return;

			actualAbilityIndex = i;
			break;
		}
	}

	//Use ability based on ability index
	switch (abilityIndex)
	{
	case 0:
		if (shieldActive)
			return;

		Shield ();
		break;
	case 1:
		if (_channelingBeam)
		{
			_channelingBeam = false;
			StopHyperBeamBP ();
		}
		else
		{
			HyperBeam ();
			return;
		}
		break;
	case 2:
		Heatseeker ();
		break;
	case 3:
		Shockwave ();
		break;
	case 4:
		Cloak ();
		break;
	case 7:
		Teleport ();
		break;
	case 8:
		Afterburner ();
		break;
	case 9:
		if (_isUsingTrapShot)
		{
			DetonateTrapShot ();
		}
		else
		{
			TrapShot (cameraPosition);
			return;
		}
		break;
	}

	//Put ability on cooldown
	_abilityCooldowns [actualAbilityIndex] = _abilityMaxCooldowns [abilityIndex];
}

bool AMainCharacterController::UseAbility_Validate (int abilityIndex, FVector cameraPosition)
{
	return true;
}

void AMainCharacterController::StartShootingInput ()
{
	if (!gameStarted || _showCursor || _dead)
		return;

	StartShooting ();
}

void AMainCharacterController::StartShooting_Implementation ()
{
	_shooting = true;
}

bool AMainCharacterController::StartShooting_Validate ()
{
	return true;
}

void AMainCharacterController::StopShooting_Implementation ()
{
	_shooting = false;
}

bool AMainCharacterController::StopShooting_Validate ()
{
	return true;
}

void AMainCharacterController::UpdateShooting ()
{
	if (_currentShootingCooldown <= 0.0f)
	{
		FVector cameraPosition = _cameraComponent->GetComponentLocation ();
		FVector cameraForward = _cameraComponent->GetForwardVector ();
		Shoot (cameraPosition, cameraForward);
	}
}

void AMainCharacterController::UpdateShootingCooldown (float deltaTime)
{
	if (_currentShootingCooldown > 0.0f)
		_currentShootingCooldown -= deltaTime;
}

void AMainCharacterController::Shoot_Implementation (FVector cameraPosition, FVector cameraForward)
{
	if (disarmed || _channelingBeam || !gameStarted || flyingIn || _power < _shootCost || _dead || _currentShootingCooldown > 0.0f || _gameFinished)
		return;

	//Reset shootingCooldown
	_currentShootingCooldown = _maxShootingCooldown;

	//Line trace from camera to check if there is something in the crosshair's sight
    FCollisionQueryParams traceParams = FCollisionQueryParams (FName (TEXT ("RV_Trace")), true, this);
    traceParams.bTraceComplex = true;
    traceParams.bReturnPhysicalMaterial = false;

    FHitResult hit (ForceInit);

	//Declare start and end position of the line trace based on camera position and rotation
	FVector start = cameraPosition;
	FVector end = cameraPosition + (cameraForward * 400000.0f);

	//Declare spawn parameters
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	FVector spawnPosition;
	FRotator spawnRotation;

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

	//Set projectile damage
	if (projectile->IsValidLowLevel () && projectile != nullptr)
		projectile->SetDamage (10 + _attackPower * 2);

	//Spend power
	_power -= _shootCost;

	ShootBP ();
}

bool AMainCharacterController::Shoot_Validate (FVector cameraPosition, FVector cameraForward)
{
	return true;
}

bool AMainCharacterController::IsAttackableInScope ()
{
	//Line trace from camera to check if there is something in the crosshair's sight
	FCollisionQueryParams traceParams = FCollisionQueryParams (FName (TEXT ("RV_Trace")), true, this);
	traceParams.bTraceComplex = true;
	traceParams.bReturnPhysicalMaterial = false;

	FHitResult hit (ForceInit);

	//Get camera position and rotation
	FVector cameraPosition = _cameraComponent->GetComponentLocation ();
	FVector cameraForward = _cameraComponent->GetForwardVector ();

	//Declare start and end position of the line trace based on camera position and rotation
	FVector start = cameraPosition;
	FVector end = cameraPosition + (cameraForward * 400000.0f);

	if (GetWorld ()->LineTraceSingleByChannel (hit, start, end, ECC_Visibility, traceParams))
	{
		//If line trace hits a projectile, spawn bullet with rotation towards the end of the line trace
		if (hit.GetActor ()->ActorHasTag ("Player") || hit.GetActor ()->ActorHasTag ("Resource") || hit.GetActor ()->ActorHasTag ("AI"))
			return true;
	}

	return false;
}

void AMainCharacterController::Shield ()
{
	//Spawn shield
	SpawnShieldBP ();
}

void AMainCharacterController::Cloak ()
{
	cloakActive = true;
	CloakBP ();
}

void AMainCharacterController::Heatseeker ()
{
	AMainCharacterController* playerTarget = nullptr;

	float distance = 0.0f;

	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
		{
			AMainCharacterController* character = Cast <AMainCharacterController> (playerController->GetCharacter ());
			
			if (character->playerID != playerID && FVector::Distance (GetActorLocation (), character->GetActorLocation ()) > distance)
				playerTarget = character;
		}
	}
	
	HeatseekerBP (playerTarget, 50 + _attackPower * 5); //Should probably change damage
}

void AMainCharacterController::Shockwave ()
{
	ShockwaveBP ();

	float hitDistance = 150.0f; //Meters

	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
		{
			AMainCharacterController* character = Cast <AMainCharacterController> (playerController->GetCharacter ());

			if (character->playerID != playerID && FVector::Distance (GetActorLocation (), character->GetActorLocation ()) <= hitDistance * 100.0f)
				character->Disarm ();
		}
	}
}

void AMainCharacterController::Disarm ()
{
	disarmed = true;

	FTimerHandle disarmTimerHandle;
	GetWorld ()->GetTimerManager ().SetTimer (disarmTimerHandle, this, &AMainCharacterController::CancelDisarm, 3.0f, false);
}

void AMainCharacterController::CancelDisarm ()
{
	disarmed = false;
}

void AMainCharacterController::Afterburner ()
{
	AfterburnerBP ();

	_isBoosting = true;

	FTimerHandle boostTimerHandle;
	GetWorld ()->GetTimerManager ().SetTimer (boostTimerHandle, this, &AMainCharacterController::CancelBoost, 3.0f, false);
}

void AMainCharacterController::TrapShot (FVector cameraPosition)
{
	_isUsingTrapShot = true;

	FVector hitPosition;

	//Line trace from camera to check if there is something in the crosshair's sight
	FCollisionQueryParams traceParams = FCollisionQueryParams (FName (TEXT ("RV_Trace")), true, this);
	traceParams.bTraceComplex = true;
	traceParams.bReturnPhysicalMaterial = false;

	FHitResult hit (ForceInit);

	//Declare start and end position of the line trace based on camera position and rotation
	FVector start = cameraPosition;
	FVector end = cameraPosition + (GetActorForwardVector () * 400000.0f);

	//Check if line trace hits anything
	if (GetWorld ()->LineTraceSingleByChannel (hit, start, end, ECC_Visibility, traceParams))
		hitPosition = hit.ImpactPoint;
	else
		hitPosition = end;

	TrapShotBP (hitPosition);
}

void AMainCharacterController::DetonateTrapShot ()
{
	_isUsingTrapShot = false;
	DetonateTrapShotBP ();

	//Get ability index
	int actualAbilityIndex = 0;

	for (int i = 0; i < _abilities.Num (); i++)
	{
		if (_abilities [i] == 9)
		{
			actualAbilityIndex = i;
			break;
		}
	}

	//Put ability on cooldown
	_abilityCooldowns [actualAbilityIndex] = _abilityMaxCooldowns [9];
}

void AMainCharacterController::StartTrapShotCooldown ()
{
	if (!_isUsingTrapShot)
		return;

	_isUsingTrapShot = false;

	//Get ability index
	int actualAbilityIndex = 0;

	for (int i = 0; i < _abilities.Num (); i++)
	{
		if (_abilities[i] == 9)
		{
			actualAbilityIndex = i;
			break;
		}
	}

	//Put ability on cooldown
	_abilityCooldowns[actualAbilityIndex] = _abilityMaxCooldowns[9];
}

void AMainCharacterController::CancelBoost ()
{
	_isBoosting = false;
}

bool AMainCharacterController::GetBoost ()
{
	return _isBoosting;
}

void AMainCharacterController::HyperBeam ()
{
	StartHyperBeamBP ();

	_channelingBeam = true;

	FTimerHandle hyperBeamTimerHandle;
	GetWorld ()->GetTimerManager ().SetTimer (hyperBeamTimerHandle, this, &AMainCharacterController::CancelHyperBeam, 5.0f, false);
}

void AMainCharacterController::ChannelHyperBeam_Implementation (FVector cameraPosition, FVector forwardVector)
{
	float damage = 60 * GetWorld ()->DeltaTimeSeconds;

	//Line trace from camera to check if there is something in the crosshair's sight
	FCollisionQueryParams traceParams = FCollisionQueryParams (FName (TEXT ("RV_Trace")), true, this);
	traceParams.bTraceComplex = true;
	traceParams.bReturnPhysicalMaterial = false;

	FHitResult hit (ForceInit);

	//Declare start and end position of the line trace based on camera position and rotation
	FVector start = cameraPosition;
	FVector end = cameraPosition + (forwardVector * 400000.0f);

	//Check if line trace hits anything
	if (GetWorld ()->LineTraceSingleByChannel (hit, start, end, ECC_Visibility, traceParams))
	{
		beamTargetPosition = hit.Location;

		//If line trace hits a player, polymorph the target
		if (hit.GetActor ()->ActorHasTag ("Player"))
		{
			AMainCharacterController* player = Cast <AMainCharacterController> (hit.GetActor ());
			player->DealBeamDamage (damage, this);
		}
	}
}

bool AMainCharacterController::ChannelHyperBeam_Validate (FVector cameraPosition, FVector forwardVector)
{
	return true;
}

void AMainCharacterController::DealBeamDamage (float damage, AMainCharacterController* player)
{
	_beamDamage += damage;

	if (_beamDamage > 1.0f)
	{
		_beamDamage -= 1.0f;

		if (shieldActive)
		{
			shield->ApplyDamage (1);
			player->UpdatePlayerHitText (player->playerID, 1);
		}
		else
		{
			_currentHealth -= 1;

			player->UpdatePlayerHitText (player->playerID, 1);

			//Register kill in game state
			if (_currentHealth <= 0 && !_dead)
			{
				AMainPlayerController* killPlayerController = Cast <AMainPlayerController> (player->GetController ());
				_gameState->AddPlayerKill (killPlayerController);

				_gameState->UpdateFeedText (_gameState->playerNames [player->playerID - 1] + " killed " + _gameState->playerNames [playerID - 1] + ".");

				Die ();
			}
		}
	}
}

void AMainCharacterController::CancelHyperBeam ()
{
	if (!_channelingBeam)
		return;

	_channelingBeam = false;

	StopHyperBeamBP ();

	//Get ability index
	int actualAbilityIndex = 0;

	for (int i = 0; i < _abilities.Num (); i++)
	{
		if (_abilities [i] == 1)
		{
			actualAbilityIndex = i;
			break;
		}
	}

	//Put ability on cooldown
	_abilityCooldowns [actualAbilityIndex] = _abilityMaxCooldowns [1];
}

bool AMainCharacterController::GetChannelingBeam ()
{
	return _channelingBeam;
}

void AMainCharacterController::Teleport ()
{
	FTimerHandle FPSTimerHandle;
	GetWorld ()->GetTimerManager ().SetTimer (FPSTimerHandle, this, &AMainCharacterController::DoTeleport, 0.1f, false); //Should be 0.5f
}

void AMainCharacterController::DoTeleport ()
{
	//Declare spawn parameters
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;

	FVector spawnPosition = GetActorLocation () + GetActorForwardVector ();
	FRotator spawnRotation = GetActorRotation ();

	//Spawn teleporter
	ATeleporter* teleporter = GetWorld ()->SpawnActor <ATeleporter> (_teleporterBP, spawnPosition, spawnRotation, spawnParams);

	//Teleport player
	teleporter->TeleportPlayer (this);
}

bool AMainCharacterController::GetShieldReflect ()
{
	return _shieldReflect;
}

void AMainCharacterController::UpdateStats (float deltaTime)
{
	//Update cooldowns
	for (int i = 0; i < _abilities.Num (); i++)
	{
		if (_abilityCooldowns [i] > 0.0f)
			_abilityCooldowns [i] -= deltaTime;
	}

	//Gradually regain power
	if (_power < _maxPower)
	{
		_power += deltaTime * (2.0f + (_attackPower / 3.5f));

		if (_power > _maxPower)
			_power = _maxPower;
	}
}

float AMainCharacterController::TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (_dead || _gameFinished)
		return 0.0f;

	float finalDamage = Damage;

	if (finalDamage > 800)
	{
		_currentHealth -= (int) finalDamage;

		shieldActive = false;

		if (shield != nullptr)
			shield->ApplyDamage ((int) finalDamage);
	}
	if (shieldActive && !DamageCauser->GetClass ()->IsChildOf (AShrinkingCircle::StaticClass ()))
	{
		finalDamage = Damage - Damage * (_defensePower * 8.0f / 100.0f);

		if (_shieldReflect && DamageCauser->GetName ().Contains ("Projectile"))
			shield->OnHitByProjectile (DamageCauser->GetOwner (), Damage);

		shield->ApplyDamage ((int) finalDamage);
		ShieldTakeDamageBP ();

		if (DamageCauser->GetName ().Contains ("Projectile"))
		{
			FString damageType = "Projectile";
			TakeDamageBP ((int) finalDamage, damageType);

			//If the player didn't get killed by AI
			if (!DamageCauser->GetOwner ()->GetClass ()->IsChildOf (ASpaceshipAI::StaticClass ()))
			{
				//Register kill in game state
				AMainCharacterController* killCharacter = Cast <AMainCharacterController> (DamageCauser->GetOwner ());

				killCharacter->UpdatePlayerHitText (killCharacter->playerID, finalDamage);
			}
		}
		else if (DamageCauser->IsA (AMainCharacterController::StaticClass ()))
		{
			//Register kill in game state
			AMainCharacterController* killCharacter = Cast <AMainCharacterController> (DamageCauser);

			if (finalDamage > 500)
				killCharacter->UpdatePlayerHitText (killCharacter->playerID, 150);
			else
				killCharacter->UpdatePlayerHitText (killCharacter->playerID, finalDamage);
		}

		return 0.0f;
	}
	else
	{
		if (!DamageCauser->GetClass ()->IsChildOf (AShrinkingCircle::StaticClass ()))
			finalDamage = Damage - Damage * (_defensePower * 8.0f / 100.0f);

			_currentHealth -= finalDamage;

		if (DamageCauser != nullptr)
		{
			if (DamageCauser->GetName ().Contains ("Projectile"))
			{
				FString damageType = "Projectile";
				TakeDamageBP ((int) finalDamage, damageType);

				//If the player didn't get killed by AI
				if (!DamageCauser->GetOwner ()->GetClass ()->IsChildOf (ASpaceshipAI::StaticClass ()))
				{
					//Register kill in game state
					AMainCharacterController* killCharacter = Cast <AMainCharacterController> (DamageCauser->GetOwner ());

					killCharacter->UpdatePlayerHitText (killCharacter->playerID, finalDamage);

					if (_currentHealth <= 0)
					{
						AMainPlayerController* killPlayerController = Cast <AMainPlayerController> (killCharacter->GetController ());
						_gameState->AddPlayerKill (killPlayerController);
						
						_gameState->UpdateFeedText (_gameState->playerNames [killCharacter->playerID - 1] + " killed " +_gameState->playerNames [playerID - 1] + ".");

						Die ();
					}
				}
			}
			else if (DamageCauser->IsA (AMainCharacterController::StaticClass ()))
			{
				//Register kill in game state
				AMainCharacterController* killCharacter = Cast <AMainCharacterController> (DamageCauser);

				if (finalDamage > 500)
					killCharacter->UpdatePlayerHitText (killCharacter->playerID, 150);
				else
					killCharacter->UpdatePlayerHitText (killCharacter->playerID, finalDamage);

				if (_currentHealth <= 0)
				{
					AMainPlayerController* killPlayerController = Cast <AMainPlayerController> (killCharacter->GetController ());
					_gameState->AddPlayerKill (killPlayerController);

					_gameState->UpdateFeedText (_gameState->playerNames [killCharacter->playerID - 1] + " killed " + _gameState->playerNames [playerID - 1] + ".");

					Die ();
				}
			}
			else if (DamageCauser->GetClass ()->IsChildOf (AShrinkingCircle::StaticClass ()))
			{
				if (_currentHealth <= 0)
				{
					_gameState->UpdateFeedText (_gameState->playerNames [playerID - 1] + " died from being outside the force field.");

					Die ();
				}
			}
			else if (DamageCauser->GetOwner ()->GetClass ()->IsChildOf (ASpaceshipAI::StaticClass ()))
			{
				if (_currentHealth <= 0)
				{
					_gameState->UpdateFeedText (_gameState->playerNames [playerID - 1] + " was killed by a turret.");

					Die ();
				}
			}
		}

		//If health is below zero, die
		if (_currentHealth <= 0 && !_dead)
			Die ();

		//Debug
		//GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, "Health: " + FString::FromInt (_currentHealth));
	}

	return Super::TakeDamage (Damage, DamageEvent, EventInstigator, DamageCauser);
}

void AMainCharacterController::UpdatePlayerHitText (int id, int damage)
{
	int damageToShow = damage;

	if (id == lastPlayerHitID && resetPlayerHitTimer > 0.0f)
		damageToShow += lastDamage;
	else
		lastDamage = 0;

	if (id == 0)
		playerHitText = "Turret\n- " + FString::FromInt (damageToShow);
	else if (id >= 100)
		playerHitText = "Resource Container\n- " + FString::FromInt (damageToShow);
	else if (id == -2)
		playerHitText = "Level Up!";
	else
		playerHitText = _gameState->playerNames [id - 1] + "\n- " + FString::FromInt (damageToShow);

	lastPlayerHitID = id;
	lastDamage = damageToShow;
	resetPlayerHitTimer = 3.0f;
	ResetPlayerHitTextBP ();
}

void AMainCharacterController::UpdatePlayerHitTextFromBP (int id, int damage)
{
	UpdatePlayerHitText (id, damage);
}

void AMainCharacterController::UpdateStatsUI ()
{
	healthPercentage = (float) _currentHealth / (float) _maxHealth;
	powerPercentage = _power / _maxPower;
	attackPowerPercentage = (float) _attackPower / (float) _maxStatPower;
	defensePowerPercentage = (float) _defensePower / (float) _maxStatPower;
	mobilityPowerPercentage = (float) _mobilityPower / (float) _maxStatPower;
	experiencePercentage = (float) _experience / (float) _experienceToNextLevel;
	healthText = FString::FromInt (_currentHealth) + "/" + FString::FromInt (_maxHealth);
	availableStats = _availableStats;
	lives = _lives;

	if (_attackUpgradesAvailable > 0)
		attackUpgradeAvailable = true;
	else
		attackUpgradeAvailable = false;

	if (_defenseUpgradesAvailable > 0)
		defenseUpgradeAvailable = true;
	else
		defenseUpgradeAvailable = false;

	if (_mobilityUpgradesAvailable > 0)
		mobilityUpgradeAvailable = true;
	else
		mobilityUpgradeAvailable = false;

	//Update ability cooldowns
	for (int i = 0; i < _hotkeyBarAbilities.Num (); i++)
	{
		int abilityIndex = _hotkeyBarAbilities [i];

		for (int j = 0; j < _abilities.Num (); j++)
		{
			if (abilityIndex == _abilities [j])
			{
				int abilityCooldownIndex = _abilities [j];
				cooldownPercentages [i] = _abilityCooldowns [j] / _abilityMaxCooldowns [abilityCooldownIndex];
			}
			else if (abilityIndex == -1)
				cooldownPercentages [i] = 0.0f;
		}
	}

	//Update respawn text
	if (_lives > 0)
	{
		if (_currentDeadTimer <= 0.0f)
			respawnText = "";
		else
			respawnText = "Respawning in " + FString::FromInt ((int) (_maxDeadTimer - _currentDeadTimer) + 1);
	}
	else
		respawnText = "Game over!\n\nLeft click to spectate other players.";
}

void AMainCharacterController::UpdateHotkeyBar (TArray <int> abilities)
{
	_hotkeyBarAbilities = abilities;
	UpdateHotkeyBarBP (_hotkeyBarAbilities);
}

void AMainCharacterController::ReplacePanelChild (UWidget* newWidget, UPanelWidget* panel, int index)
{
	panel->ReplaceChildAt (index, newWidget);
}

void AMainCharacterController::SwitchPanelPosition (UWidget* widgetOne, UWidget* widgetTwo, UPanelWidget* panel)
{
	UUserWidget* newWidgetOne = CreateWidget (panel, widgetOne->GetClass ());
	UUserWidget* newWidgetTwo = CreateWidget (panel, widgetTwo->GetClass ());

	int childOneIndex = panel->GetChildIndex (widgetOne);
	int childTwoIndex = panel->GetChildIndex (widgetTwo);

	panel->ReplaceChildAt (childOneIndex, newWidgetTwo);
	panel->ReplaceChildAt (childTwoIndex, newWidgetOne);
}

int AMainCharacterController::GetPlayerID ()
{
	return playerID;
}

void AMainCharacterController::EnableMouseCursor ()
{
	if (_inSettingsMenu)
		return;

	if (!GetWorld ()->IsServer () && IsLocallyControlled ())
	{
		GetWorld ()->GetFirstPlayerController ()->bShowMouseCursor = true;
		GetWorld ()->GetFirstPlayerController ()->bEnableClickEvents = true;
		GetWorld ()->GetFirstPlayerController ()->bEnableMouseOverEvents = true;

		if (GetWorld ()->WorldType == EWorldType::Game)
		{
			//Set input mode to UI
			FInputModeGameAndUI uiInputMode;
			GetWorld ()->GetFirstPlayerController ()->SetInputMode (uiInputMode);
		}

		//Center mouse position
		FVector2D viewPort = GetViewportSize();
		GetWorld()->GetFirstPlayerController()->SetMouseLocation(viewPort.X / 2, viewPort.Y / 2);

		//Show cursor
		SetShowCursor (true);

		StopShooting ();
	}
}

void AMainCharacterController::DisableMouseCursor ()
{
	if (_inSettingsMenu)
		return;

	if (!GetWorld ()->IsServer () && IsLocallyControlled () && !_inAbilityMenu)
	{
		GetWorld ()->GetFirstPlayerController ()->bShowMouseCursor = false;
		GetWorld ()->GetFirstPlayerController ()->bEnableClickEvents = false;
		GetWorld ()->GetFirstPlayerController ()->bEnableMouseOverEvents = false;

		if (GetWorld ()->WorldType == EWorldType::Game)
		{
			//Set input mode to game
			FInputModeGameOnly gameInputMode;
			GetWorld ()->GetFirstPlayerController ()->SetInputMode (gameInputMode);
		}

		SetShowCursor (false);
	}
}

void AMainCharacterController::SetShowCursor_Implementation (bool show)
{
	_showCursor = show;
}

bool AMainCharacterController::SetShowCursor_Validate (bool show)
{
	return true;
}

void AMainCharacterController::MouseClick ()
{
	if (_gameFinished)
		return;

	if (_gameOver)
	{
		if (!isSpectating)
		{
			StartSpectatingBP ();
			isSpectating = true;
		}
		else
			ChangeSpectateTargetBP ();
	}

	if (!_showCursor)
		return;

	//Trace to see what is under the mouse cursor
	FHitResult hit;

	if (GetWorld ()->GetFirstPlayerController ()->GetHitResultUnderCursor (ECC_Visibility, true, hit))
	{
		FString hitName = hit.GetComponent ()->GetName ();

		if (hitName == "AttackUpgradeButton")
			AddStat (1);
		else if (hitName == "DefenseUpgradeButton")
			AddStat (2);
		else if (hitName == "MobilityUpgradeButton")
			AddStat (3);
		else if (hitName == "AbilityMenuButton")
			ToggleAbilityMenu ();
	}
}

void AMainCharacterController::ToggleAbilityMenu ()
{
	if (_inSettingsMenu)
		return;

	if (_inAbilityMenu)
		CloseAbilityMenu ();
	else
	{
		OpenAbilityMenuBP ();

		if (!_showCursor)
			EnableMouseCursor ();

		_inAbilityMenu = true;
	}
}

void AMainCharacterController::CloseAbilityMenu ()
{
	_inAbilityMenu = false;
	DisableMouseCursor ();

	CloseAbilityMenuBP ();
}

void AMainCharacterController::OpenSettingsMenu (bool open)
{
	if (open)
	{
		EnableMouseCursor ();
		_inSettingsMenu = true;
	}
	else
	{
		_inSettingsMenu = false;
		DisableMouseCursor ();
	}
}

bool AMainCharacterController::GetCanMove ()
{
	if (_dead || _showCursor || !gameStarted || _gameFinished)
		return false;

	return true;
}

bool AMainCharacterController::GetIsDead ()
{
	if (!gameStarted || _dead || _gameFinished)
		return true;

	return _dead;
}

int AMainCharacterController::GetMobilityPower ()
{
	return _mobilityPower;
}

void AMainCharacterController::FinishFlyingIn ()
{
	flyingIn = false;
	Cast <AMainPlayerController> (GetController ())->flyingIn = false;
}

void AMainCharacterController::SetShieldHealth_Implementation (int health)
{
	shieldHealth = health;
}

//Returns the viewport of the client
FVector2D AMainCharacterController::GetViewportSize() 
{
	FVector2D result = FVector2D(1.0f,1.0f);

	if (GEngine != NULL && GEngine->GameViewport != NULL)
	{
		GEngine->GameViewport->GetViewportSize(result);
	}
	return result;
}

void AMainCharacterController::StartGame ()
{
	gameStarted = true;
}

void AMainCharacterController::GetLifetimeReplicatedProps (TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps (OutLifetimeProps);

	DOREPLIFETIME (AMainCharacterController, _maxHealth);
	DOREPLIFETIME (AMainCharacterController, _currentHealth);
	DOREPLIFETIME (AMainCharacterController, _maxPower);
	DOREPLIFETIME (AMainCharacterController, _power);
	DOREPLIFETIME (AMainCharacterController, _attackPower);
	DOREPLIFETIME (AMainCharacterController, _defensePower);
	DOREPLIFETIME (AMainCharacterController, _mobilityPower);
	DOREPLIFETIME (AMainCharacterController, _dead);
	DOREPLIFETIME (AMainCharacterController, _experience);
	DOREPLIFETIME (AMainCharacterController, _experienceToNextLevel);
	DOREPLIFETIME (AMainCharacterController, _availableStats);
	DOREPLIFETIME (AMainCharacterController, _shooting);
	DOREPLIFETIME (AMainCharacterController, _currentShootingCooldown);
	DOREPLIFETIME (AMainCharacterController, playerHitText);

	DOREPLIFETIME (AMainCharacterController, _attackUpgradesAvailable);
	DOREPLIFETIME (AMainCharacterController, _defenseUpgradesAvailable);
	DOREPLIFETIME (AMainCharacterController, _mobilityUpgradesAvailable);
	
	DOREPLIFETIME (AMainCharacterController, _abilities);
	DOREPLIFETIME (AMainCharacterController, _abilityCooldowns);

	DOREPLIFETIME (AMainCharacterController, _currentDeadTimer);
	DOREPLIFETIME (AMainCharacterController, _lives);
	DOREPLIFETIME (AMainCharacterController, _gameOver);

	DOREPLIFETIME (AMainCharacterController, _showCursor);

	DOREPLIFETIME (AMainCharacterController, shieldActive);

	DOREPLIFETIME (AMainCharacterController, playerID);
	DOREPLIFETIME (AMainCharacterController, gameStarted);

	DOREPLIFETIME (AMainCharacterController, systemLevel);

	DOREPLIFETIME (AMainCharacterController, _channelingBeam);
	DOREPLIFETIME (AMainCharacterController, beamTargetPosition);	

	DOREPLIFETIME (AMainCharacterController, disarmed);

	DOREPLIFETIME (AMainCharacterController, flyingIn);
	DOREPLIFETIME (AMainCharacterController, _gameFinished);
}

//Called to bind functionality to input
void AMainCharacterController::SetupPlayerInputComponent (UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent (PlayerInputComponent);

	//Set up shoot bindings
    PlayerInputComponent->BindAction ("Shoot", IE_Pressed, this, &AMainCharacterController::StartShootingInput);
	PlayerInputComponent->BindAction ("Shoot", IE_Released, this, &AMainCharacterController::StopShooting);

	//Set up "add stat" bindings
	PlayerInputComponent->BindAction ("AddAttackStat", IE_Pressed, this, &AMainCharacterController::AddStat <1>);
	PlayerInputComponent->BindAction ("AddDefenseStat", IE_Pressed, this, &AMainCharacterController::AddStat <2>);
	PlayerInputComponent->BindAction ("AddMobilityStat", IE_Pressed, this, &AMainCharacterController::AddStat <3>);

	//Set up ability bindings
	PlayerInputComponent->BindAction ("Ability1", IE_Pressed, this, &AMainCharacterController::UseAbilityInput <1>);
	PlayerInputComponent->BindAction ("Ability2", IE_Pressed, this, &AMainCharacterController::UseAbilityInput <2>);
	PlayerInputComponent->BindAction ("Ability3", IE_Pressed, this, &AMainCharacterController::UseAbilityInput <3>);
	PlayerInputComponent->BindAction ("Ability4", IE_Pressed, this, &AMainCharacterController::UseAbilityInput <4>);
	PlayerInputComponent->BindAction ("Ability5", IE_Pressed, this, &AMainCharacterController::UseAbilityInput <5>);
	PlayerInputComponent->BindAction ("Ability6", IE_Pressed, this, &AMainCharacterController::UseAbilityInput <6>);
	PlayerInputComponent->BindAction ("Ability7", IE_Pressed, this, &AMainCharacterController::UseAbilityInput <7>);
	PlayerInputComponent->BindAction ("Ability8", IE_Pressed, this, &AMainCharacterController::UseAbilityInput <8>);

	//Set up mouse cursor bindings
	PlayerInputComponent->BindAction ("ShowMouseCursor", IE_Pressed, this, &AMainCharacterController::EnableMouseCursor);
	PlayerInputComponent->BindAction ("ShowMouseCursor", IE_Released, this, &AMainCharacterController::DisableMouseCursor);
	PlayerInputComponent->BindAction ("MouseClick", IE_Pressed, this, &AMainCharacterController::MouseClick);

	//Set up menu bindings
	PlayerInputComponent->BindAction ("AbilityMenu", IE_Pressed, this, &AMainCharacterController::ToggleAbilityMenu);
}
