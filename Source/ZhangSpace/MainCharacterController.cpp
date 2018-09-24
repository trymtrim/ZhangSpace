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
		TArray <UStaticMeshComponent*> meshComps;
		GetComponents <UStaticMeshComponent> (meshComps);
		UStaticMeshComponent* meshComponent;

		if (GetWorld ()->WorldType == EWorldType::Game)
			meshComponent = meshComps [0];
		else
			meshComponent = meshComps [1];

		meshComponent->SetStaticMesh (_cockpitMesh);
	}
}

//Called every frame
void AMainCharacterController::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	//Update cooldowns server side
	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
		UpdateStats (DeltaTime);

	//Update stats for the client's UI
	if (!GetWorld ()->IsServer () && IsLocallyControlled ())
		UpdateStatsUI ();
}

void AMainCharacterController::MoveForward (float value)
{
	if (value != 0.0f)
	{
		//Add movement in that direction
		AddMovementInput (GetActorForwardVector (), value);
	}
}

void AMainCharacterController::MoveRight (float value)
{
	if (value != 0.0f)
	{
		//Add movement in that direction
		AddMovementInput (GetActorRightVector (), value);
	}
}

void AMainCharacterController::Shoot_Implementation ()
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
	FVector spawnPosition = GetActorLocation () + GetActorForwardVector () * 200.0f;
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

	AProjectile* projectile = GetWorld ()->SpawnActor <AProjectile> (_projectileBP, spawnPosition, spawnRotation, spawnParams);

	if (projectile->IsValidLowLevel () && projectile != nullptr)
		projectile->SetDamage (_attackPower);

	//Spend power
	_power -= _shootCost;
}

bool AMainCharacterController::Shoot_Validate ()
{
    return true;
}

void AMainCharacterController::Shield_Implementation ()
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

bool AMainCharacterController::Shield_Validate ()
{
	return true;
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
	shieldCooldownPercentage = ((float) _maxShieldCooldown - (float) _currentShieldCooldown) / (float) _maxShieldCooldown;
	healthText = FString::FromInt (_currentHealth) + "/" + FString::FromInt (_maxHealth);
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
	DOREPLIFETIME (AMainCharacterController, _maxShieldCooldown);
	DOREPLIFETIME (AMainCharacterController, _currentShieldCooldown);
	DOREPLIFETIME (AMainCharacterController, _dead);
}

//Called to bind functionality to input
void AMainCharacterController::SetupPlayerInputComponent (UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent (PlayerInputComponent);

	//Set up movement bindings
    PlayerInputComponent->BindAxis ("MoveForward", this, &AMainCharacterController::MoveForward);
    PlayerInputComponent->BindAxis ("MoveRight", this, &AMainCharacterController::MoveRight);

	//Set up action bindings
    PlayerInputComponent->BindAction ("Shoot", IE_Pressed, this, &AMainCharacterController::Shoot);
	PlayerInputComponent->BindAction ("Shield", IE_Pressed, this, &AMainCharacterController::Shield);
}
