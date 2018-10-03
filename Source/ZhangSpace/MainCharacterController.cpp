// Copyright Team Monkey Business 2018.

#include "MainCharacterController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "UnrealNetwork.h"

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
		AddAbility (1);
	}
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
	AddActorLocalRotation(_playerDeltaRotation, false, 0, ETeleportType::None);

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Rotation: " + _playerDeltaRotation.ToString());

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

	if (abilityIndex <= 3)
		_attackUpgradeAvailable = false;
	else if (abilityIndex > 3 && abilityIndex <= 6)
		_defenseUpgradeAvailable = false;
	else if (abilityIndex > 6 && abilityIndex <= 9)
		_mobilityUpgradeAvailable = false;
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
			_attackUpgradeAvailable = true;
		break;
	case 2: //Defense
		_defensePower++;

		if (_defensePower == 3 || _defensePower == 7 || _defensePower == 10)
			_defenseUpgradeAvailable;
		break;
	case 3: //Mobility
		_mobilityPower++;

		if (_mobilityPower == 3 || _mobilityPower == 7 || _mobilityPower == 10)
			_mobilityUpgradeAvailable = true;
		break;
	}
}

bool AMainCharacterController::AddStat_Validate (int statIndex)
{
	return true;
}

void AMainCharacterController::UseAbilityInput (int abilityIndex)
{
	//TODO: Use the ability that is assigned to this slot if the player has the ability

	//For now
	int actualAbilityIndex = abilityIndex;

	if (_abilities.Contains (actualAbilityIndex))
		UseAbility (actualAbilityIndex);
}

void AMainCharacterController::UseAbility_Implementation (int abilityIndex)
{
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
	FVector spawnPosition = GetActorLocation () + GetActorForwardVector () * 350.0f;
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
	if (_currentShieldCooldown > 0.0f || _dead)
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

	attackUpgradeAvailable = _attackUpgradeAvailable;
	defenseUpgradeAvailable = _defenseUpgradeAvailable;
	mobilityUpgradeAvailable = _mobilityUpgradeAvailable;

	shieldCooldownPercentage = (float) _currentShieldCooldown / (float) _maxShieldCooldown;
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

	DOREPLIFETIME (AMainCharacterController, _attackUpgradeAvailable);
	DOREPLIFETIME (AMainCharacterController, _defenseUpgradeAvailable);
	DOREPLIFETIME (AMainCharacterController, _mobilityUpgradeAvailable);

	DOREPLIFETIME (AMainCharacterController, _maxShieldCooldown);
	DOREPLIFETIME (AMainCharacterController, _currentShieldCooldown);
	DOREPLIFETIME (AMainCharacterController, _playerDeltaRotation);
}

//Called to bind functionality to input
void AMainCharacterController::SetupPlayerInputComponent (UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent (PlayerInputComponent);

	//Set up shoot bindings
    PlayerInputComponent->BindAction ("Shoot", IE_Pressed, this, &AMainCharacterController::StartShooting);
	PlayerInputComponent->BindAction ("Shoot", IE_Released, this, &AMainCharacterController::StopShooting);

	//Set up "add stat" bindings
	PlayerInputComponent->BindAction ("AddAttackStat", IE_Pressed, this, &AMainCharacterController::AddStat <1>);
	PlayerInputComponent->BindAction ("AddDefenseStat", IE_Pressed, this, &AMainCharacterController::AddStat <2>);
	PlayerInputComponent->BindAction ("AddMobilityStat", IE_Pressed, this, &AMainCharacterController::AddStat <3>);

	//Set up ability bindings
	PlayerInputComponent->BindAction ("Ability1", IE_Pressed, this, &AMainCharacterController::UseAbilityInput <1>);
}
