// Copyright Team Monkey Business 2018.

#include "MainCharacterController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Components/ArrowComponent.h"

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

		//Add shield ability
		AddAbility (0);

		//Temp
		//AddAbility (7);
	}

	if (GetWorld ()->IsServer ())
	{
		//Debugging
		/*AddExperience (100);
		AddExperience (100);
		AddExperience (100);
		AddExperience (100);
		AddExperience (100);
		AddExperience (100);
		AddExperience (100);
		AddExperience (100);
		AddExperience (100);
		AddExperience (100);
		AddExperience (100);
		AddExperience (100);*/
		//AddAbility (4);
		//AddAbility (7);
	}

	//FPS
	if (!GetWorld ()->IsServer () && IsLocallyControlled ())
	{
		FTimerHandle FPSTimerHandle;
		GetWorld ()->GetTimerManager ().SetTimer (FPSTimerHandle, this, &AMainCharacterController::UpdateFPS, 1.0f, true);
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
	}

	//Update stats for the client's UI
	if (!GetWorld ()->IsServer () && IsLocallyControlled ())
	{
		UpdateStatsUI ();

		if (_shooting)
			UpdateShooting ();

		FPS++;
	}
}

void AMainCharacterController::InitializeAbilityCooldowns ()
{
	_abilityMaxCooldowns.Add (0, 60.0f); //Shield
	_abilityMaxCooldowns.Add (7, 15.0f); //Teleport

	//Add shield ability to hotkey bar //TEMP
	_hotkeyBarAbilities.Add (0); //Probably not temp
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

void AMainCharacterController::Die ()
{
	//Update lives and health
	_lives--;
	_currentHealth = 0;
	_dead = true;

	DieBP ();

	//Stop movement
	GetCharacterMovement ()->StopMovementImmediately ();

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
	TArray <AActor*> playerStarts;
	UGameplayStatics::GetAllActorsOfClass (GetWorld (), APlayerStart::StaticClass (), playerStarts);

	int randomIndex = FMath::RandRange (0, playerStarts.Num () - 1);
	SetActorLocation (playerStarts [randomIndex]->GetActorLocation ());

	RespawnBP ();

	_dead = false;
}

void AMainCharacterController::GameOver ()
{
	//Do stuff
}

void AMainCharacterController::AddExperience (int experience)
{
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
	}
}

void AMainCharacterController::AddAbility (int abilityIndex)
{
	ServerAddAbility (abilityIndex);

	//Temp for midterm
	if (abilityIndex == 7)
	{
		teleportUnlocked = true;
		_hotkeyBarAbilities.Add (7);
	}
}

void AMainCharacterController::ServerAddAbility_Implementation (int abilityIndex)
{
	//If the ability is a passive, enable the respective bool, otherwise add it the the ability list
	if (abilityIndex == 4) //||...)
	{
		switch (abilityIndex)
		{
		case 4: //Shield reflect
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

	_availableStats--;

	switch (statIndex)
	{
	case 1: //Attack
		_attackPower++;

		_maxShootingCooldown -= _maxShootingCooldown * 0.1f;

		//GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, FString::SanitizeFloat (_maxShootingCooldown));

		if (_attackPower == 3 || _attackPower == 6 || _attackPower == 10)
			_attackUpgradesAvailable++;
		break;
	case 2: //Defense
		_defensePower++;

		if (_defensePower == 3 || _defensePower == 6 || _defensePower == 10)
			_defenseUpgradesAvailable++;
		break;
	case 3: //Mobility
		_mobilityPower++;

		if (_mobilityPower == 3 || _mobilityPower == 6 || _mobilityPower == 10)
			_mobilityUpgradesAvailable++;
		break;
	}
}

bool AMainCharacterController::AddStat_Validate (int statIndex)
{
	return true;
}

void AMainCharacterController::UseAbilityInput (int abilityIndex)
{
	if (_dead || _showCursor || _hotkeyBarAbilities.Num () < abilityIndex)
		return;

	//Get camera component
	TArray <UCameraComponent*> cameraComps;
	GetComponents <UCameraComponent> (cameraComps);
	UCameraComponent* cameraComponent = cameraComps [0];

	FVector cameraPosition = cameraComponent->GetComponentLocation ();

	int actualAbilityIndex = _hotkeyBarAbilities [abilityIndex -1];
	UseAbility (actualAbilityIndex, cameraPosition);
}

void AMainCharacterController::UseAbility_Implementation (int abilityIndex, FVector cameraPosition)
{
	if (!_abilities.Contains (abilityIndex))
		return;

	//Put ability on cooldown
	for (int i = 0; i < _abilities.Num (); i++)
	{
		if (_abilities [i] == abilityIndex)
		{
			if (_abilityCooldowns [i] > 0.0f)
				return;


			_abilityCooldowns [i] = _abilityMaxCooldowns [abilityIndex];
			break;
		}
	}

	//Use ability based on ability index
	switch (abilityIndex)
	{
	case 0:
		Shield ();
		break;
	case 7:
		Teleport ();
		break;
	}
}

bool AMainCharacterController::UseAbility_Validate (int abilityIndex, FVector cameraPosition)
{
	return true;
}

void AMainCharacterController::StartShootingInput ()
{
	if (_showCursor)
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
		//Get camera component
		TArray <UCameraComponent*> cameraComps;
		GetComponents <UCameraComponent> (cameraComps);
		UCameraComponent* cameraComponent = cameraComps[0];

		FVector cameraPosition = cameraComponent->GetComponentLocation ();
		FVector cameraForward = cameraComponent->GetForwardVector ();
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
	if (_power < _shootCost || _dead || _currentShootingCooldown > 0.0f)
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
	FVector spawnPosition; // = GetActorLocation () + GetActorForwardVector () * 350.0f - GetActorUpVector () * 35.0f;
	FRotator spawnRotation;

	//Spawn projectile at assigned gun position
	TArray <UArrowComponent*> arrowComps;
	GetComponents <UArrowComponent> (arrowComps);
	
	if (GetWorld ()->WorldType == EWorldType::Game)
	{
		if (gunPositionSwitch)
			spawnPosition = arrowComps [0]->GetComponentLocation ();
		else
			spawnPosition = arrowComps [1]->GetComponentLocation ();
	}
	else
	{
		if (gunPositionSwitch)
			spawnPosition = arrowComps [1]->GetComponentLocation ();
		else
			spawnPosition = arrowComps [2]->GetComponentLocation ();
	}
		
	gunPositionSwitch = !gunPositionSwitch;

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
		projectile->SetDamage (10 + _attackPower * 3);

	//Spend power
	_power -= _shootCost;

	ShootBP ();
}

bool AMainCharacterController::Shoot_Validate (FVector cameraPosition, FVector cameraForward)
{
	return true;
}

void AMainCharacterController::Shield ()
{
	//Spawn shield
	SpawnShieldBP ();
}

void AMainCharacterController::Teleport ()
{
	//Declare spawn parameters
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	FVector spawnPosition = GetActorLocation () + GetActorForwardVector () * 5000.0f;
	FRotator spawnRotation = GetActorRotation ();

	//Spawn projectile
	GetWorld ()->SpawnActor <AActor> (_teleporterBP, spawnPosition, spawnRotation, spawnParams);
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
	if (_dead)
		return 0.0f;

	if (shieldActive && !DamageCauser->GetClass ()->IsChildOf (AShrinkingCircle::StaticClass ()))
	{
		shield->ApplyDamage (Damage);
		ShieldTakeDamageBP ();

		return 0.0f;
	}
	else
	{
		_currentHealth -= Damage;

		if (DamageCauser != nullptr)
		{
			if (DamageCauser->GetName ().Contains ("Projectile"))
			{
				FString damageType = "Projectile";
				TakeDamageBP ((int) Damage, damageType);
			}
		}

		//If health is below zero, die
		if (_currentHealth <= 0)
			Die ();

		//Debug
		//GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, "Health: " + FString::FromInt (_currentHealth));
	}

	return Super::TakeDamage (Damage, DamageEvent, EventInstigator, DamageCauser);
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
		respawnText = "Game over!";
}

void AMainCharacterController::UpdateHotkeyBar (TArray <int> abilities)
{
	_hotkeyBarAbilities = abilities;
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
	if (_dead || _showCursor)
		return false;

	return true;
}

bool AMainCharacterController::GetIsDead ()
{
	return _dead;
}

int AMainCharacterController::GetMobilityPower ()
{
	return _mobilityPower;
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

	DOREPLIFETIME (AMainCharacterController, _attackUpgradesAvailable);
	DOREPLIFETIME (AMainCharacterController, _defenseUpgradesAvailable);
	DOREPLIFETIME (AMainCharacterController, _mobilityUpgradesAvailable);
	
	DOREPLIFETIME (AMainCharacterController, _abilities);
	DOREPLIFETIME (AMainCharacterController, _abilityCooldowns);

	DOREPLIFETIME (AMainCharacterController, _currentDeadTimer);
	DOREPLIFETIME (AMainCharacterController, _lives);

	DOREPLIFETIME (AMainCharacterController, _showCursor);

	DOREPLIFETIME (AMainCharacterController, shieldActive);
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

	//Set up mouse cursor bindings
	PlayerInputComponent->BindAction ("ShowMouseCursor", IE_Pressed, this, &AMainCharacterController::EnableMouseCursor);
	PlayerInputComponent->BindAction ("ShowMouseCursor", IE_Released, this, &AMainCharacterController::DisableMouseCursor);
	PlayerInputComponent->BindAction ("MouseClick", IE_Pressed, this, &AMainCharacterController::MouseClick);

	//Set up menu bindings
	PlayerInputComponent->BindAction ("AbilityMenu", IE_Pressed, this, &AMainCharacterController::ToggleAbilityMenu);
}
