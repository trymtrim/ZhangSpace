// Copyright Team Monkey Business 2018.

#include "MainCharacterController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "UnrealNetwork.h"
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

	//Change mesh on client-side
	if (!GetWorld ()->IsServer () && IsLocallyControlled ())
	{
		ChangeMesh ();
	}

	if (GetWorld ()->IsServer ())
		AddAbility (1);
}

//Called every frame
void AMainCharacterController::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	//Update cooldowns server side
	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
	{
		UpdateShooting (DeltaTime);
		UpdateStats (DeltaTime);
	}

	//Update stats for the client's UI
	if (!GetWorld ()->IsServer () && IsLocallyControlled ())
		UpdateStatsUI ();

	//Update local rotation based on delta rotation in MainPlayerController class

	//Note: Offset after some gameplay time...
	//Solution: Add local rotation in playercontroller and then set actor rotation here

	//AddActorLocalRotation (_playerRotation, false, 0, ETeleportType::None);

	SetActorRotation(_playerRotation,ETeleportType::None);

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Rotation: " + _playerRotation.ToString());

}

void AMainCharacterController::ChangeMesh ()
{
	TArray <UStaticMeshComponent*> meshComps;
	GetComponents <UStaticMeshComponent> (meshComps);
	UStaticMeshComponent* meshComponent;

	if (GetWorld ()->WorldType == EWorldType::Game)
		meshComponent = meshComps [0];
	else
		meshComponent = meshComps [1];

	meshComponent->SetStaticMesh (_cockpitMesh);
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
	_abilities.Add (abilityIndex);

	if (_attackUpgradesAvailable != 0 && _defenseUpgradesAvailable != 0 && _mobilityUpgradesAvailable != 0)
	{
		if (abilityIndex <= 3)
			_attackUpgradesAvailable--;
		else if (abilityIndex > 3 && abilityIndex <= 6)
			_defenseUpgradesAvailable--;
		else if (abilityIndex > 6 && abilityIndex <= 9)
			_mobilityUpgradesAvailable--;
	}
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

		if (_attackPower == 3 || _attackPower == 7 || _attackPower == 10)
			_attackUpgradesAvailable++;
		break;
	case 2: //Defense
		_defensePower++;

		if (_defensePower == 3 || _defensePower == 7 || _defensePower == 10)
			_defenseUpgradesAvailable++;
		break;
	case 3: //Mobility
		_mobilityPower++;

		if (_mobilityPower == 3 || _mobilityPower == 7 || _mobilityPower == 10)
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
	if (_dead || _showCursor)
		return;

	//TODO: Use the ability that is assigned to this slot if the player has the ability

	//For now
	int actualAbilityIndex = abilityIndex;

	UseAbility (actualAbilityIndex);
}

void AMainCharacterController::UseAbility_Implementation (int abilityIndex)
{
	if (!_abilities.Contains (abilityIndex))
		return;

	switch (abilityIndex)
	{
	case 1:
		Shield ();
		break;
	}
}

bool AMainCharacterController::UseAbility_Validate (int abilityIndex)
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

void AMainCharacterController::UpdateShooting (float deltaTime)
{
	if (_currentShootingCooldown > 0.0f)
		_currentShootingCooldown -= deltaTime;
	else if (_shooting)
	{
		_currentShootingCooldown = _maxShootingCooldown;
		Shoot ();
	}
}

void AMainCharacterController::Shoot ()
{
	if (_power < _shootCost || _dead)
		return;

	//Line trace from camera to check if there is something in the crosshair's sight
    FCollisionQueryParams traceParams = FCollisionQueryParams (FName (TEXT ("RV_Trace")), true, this);
    traceParams.bTraceComplex = true;
    traceParams.bReturnPhysicalMaterial = false;

    FHitResult hit (ForceInit);
	
	//Get camera component
	TArray <UCameraComponent*> cameraComps;
	GetComponents <UCameraComponent> (cameraComps);
	UCameraComponent* cameraComponent = cameraComps [0];

	FVector cameraPosition = cameraComponent->GetComponentLocation ();

	//Declare start and end position of the line trace based on camera position and rotation
	FVector start = cameraPosition;
	FVector end = cameraPosition + (cameraComponent->GetForwardVector () * 10000.0f);

	//Declare spawn parameters
	FActorSpawnParameters spawnParams;
	FVector spawnPosition = GetActorLocation () + GetActorForwardVector () * 350.0f - GetActorUpVector () * 40.0f;
	FRotator spawnRotation;

	//Check if line trace hits anything
    if (GetWorld ()->LineTraceSingleByChannel (hit, start, end, ECC_Visibility, traceParams))
    {
		//If line trace hits a projectile, spawn bullet with rotation towards the end of the line trace
		if (hit.GetActor ()->ActorHasTag ("Projectile"))
			spawnRotation = (end - GetActorLocation ()).Rotation ();
		else //Otherwise, spawn bullet with rotation towards what it hits
			spawnRotation = (hit.ImpactPoint - GetActorLocation ()).Rotation ();
    }
    else //If line trace doesn't hit anything, spawn bullet with rotation towards the end of the line trace
        spawnRotation = (end - GetActorLocation ()).Rotation ();

	//Spawn projectile
	AProjectile* projectile = GetWorld ()->SpawnActor <AProjectile> (_projectileBP, spawnPosition, spawnRotation, spawnParams);

	//Set projectile damage
	if (projectile->IsValidLowLevel () && projectile != nullptr)
		projectile->SetDamage (_attackPower * 10);

	//Spend power
	_power -= _shootCost;
}

void AMainCharacterController::Shield ()
{
	if (_currentShieldCooldown > 0.0f)
		return;

	//Reset shield cooldown
	_currentShieldCooldown = _maxShieldCooldown;

	//Declare spawn parameters
	FActorSpawnParameters spawnParams;
	FVector spawnPosition = GetActorLocation ();
	FRotator spawnRotation = FRotator (0.0f, 0.0f, 0.0f);

	//Spawn shield
	AShield* shield = GetWorld ()->SpawnActor <AShield> (_shieldBP, spawnPosition, spawnRotation, spawnParams);
	shield->SetShieldOwner (this);
}

void AMainCharacterController::UpdateStats (float deltaTime)
{
	//Shield cooldown
	if (_currentShieldCooldown > 0.0f)
		_currentShieldCooldown -= deltaTime;

	//Gradually regain power
	if (_power < _maxPower)
	{
		_power += deltaTime;

		if (_power > _maxPower)
			_power = _maxPower;
	}
}

float AMainCharacterController::TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (_dead)
		return 0.0f;

	_currentHealth -= Damage;

	//If health is below zero, die
	if (_currentHealth <= 0)
	{
		_currentHealth = 0;
		_dead = true;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, "Health: " + FString::FromInt (_currentHealth));

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

	shieldCooldownPercentage = (float) _currentShieldCooldown / (float) _maxShieldCooldown;
}

void AMainCharacterController::EnableMouseCursor ()
{
	if (!GetWorld ()->IsServer () && IsLocallyControlled ())
	{
		GetWorld ()->GetFirstPlayerController ()->bShowMouseCursor = true;
		GetWorld ()->GetFirstPlayerController ()->bEnableClickEvents = true;
		GetWorld ()->GetFirstPlayerController ()->bEnableMouseOverEvents = true;

		//Center mouse position - Ari
		FVector2D viewPort = GetViewportSize();
		GetWorld()->GetFirstPlayerController()->SetMouseLocation(viewPort.X / 2, viewPort.Y / 2);

		//Debug that shit
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Viewport: " + FString::SanitizeFloat(viewPort.X) + ", " + FString::SanitizeFloat(viewPort.Y));
		
		//Show cursor
		_showCursor = true;

		StopShooting ();
	}
}

void AMainCharacterController::DisableMouseCursor ()
{
	if (!GetWorld ()->IsServer () && IsLocallyControlled () && !_inAbilityMenu)
	{
		GetWorld ()->GetFirstPlayerController ()->bShowMouseCursor = false;
		GetWorld ()->GetFirstPlayerController ()->bEnableClickEvents = false;
		GetWorld ()->GetFirstPlayerController ()->bEnableMouseOverEvents = false;

		_showCursor = false;
	}
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
	if (_inAbilityMenu)
		CloseAbilityMenu ();
	else
	{
		OpenAbilityMenuBP ();

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

	DOREPLIFETIME (AMainCharacterController, _attackUpgradesAvailable);
	DOREPLIFETIME (AMainCharacterController, _defenseUpgradesAvailable);
	DOREPLIFETIME (AMainCharacterController, _mobilityUpgradesAvailable);

	DOREPLIFETIME (AMainCharacterController, _maxShieldCooldown);
	DOREPLIFETIME (AMainCharacterController, _currentShieldCooldown);
	DOREPLIFETIME (AMainCharacterController, _playerRotation);
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

	//Set up mouse cursor bindings
	PlayerInputComponent->BindAction ("ShowMouseCursor", IE_Pressed, this, &AMainCharacterController::EnableMouseCursor);
	PlayerInputComponent->BindAction ("ShowMouseCursor", IE_Released, this, &AMainCharacterController::DisableMouseCursor);
	PlayerInputComponent->BindAction ("MouseClick", IE_Pressed, this, &AMainCharacterController::MouseClick);

	//Set up menu bindings
	PlayerInputComponent->BindAction ("AbilityMenu", IE_Pressed, this, &AMainCharacterController::ToggleAbilityMenu);
}
