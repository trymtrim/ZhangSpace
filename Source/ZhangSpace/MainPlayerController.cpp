// Copyright Team Monkey Business 2018.

#include "MainPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "UnrealNetwork.h"
#include "Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "MainGameState.h"
#include "ConfigManager.h"
#include "SettingsManager.h"

AMainPlayerController::AMainPlayerController()
{
	//Set this character to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Lock mouse to viewport when starting a game
	if (GetWorld()->GetGameViewport() != nullptr)
	{
		GetWorld()->GetGameViewport()->SetMouseLockMode(EMouseLockMode::LockAlways);
		GetWorld()->GetGameViewport()->Viewport->LockMouseToViewport(true);
	}

	PlayerCameraManager->ViewPitchMax = 359.998993f;
	PlayerCameraManager->ViewPitchMin = 0.0f;
	PlayerCameraManager->ViewYawMax = 359.998993f;
	PlayerCameraManager->ViewRollMax = 359.998993f;
	PlayerCameraManager->ViewRollMin = 0.0f;

	if (!GetWorld ()->IsServer ())
		RegisterPlayer (ConfigManager::GetConfig ("Player_Name"), Cast <USettingsManager> (GetWorld ()->GetGameInstance ())->GetTargetPlayerCount ());

	//To get mobility power (values = 1-10):
	//_character->GetMobilityPower ();
}

//Called every frame
void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_character == nullptr) 
	{
		_character = Cast <AMainCharacterController>(GetCharacter());

		if (_UCharMoveComp == nullptr && _character != nullptr)
			_UCharMoveComp = _character->GetCharacterMovement();
	}

	if (!GetWorld ()->IsServer ())
		UpdatePlayerRotation (pitchDelta, yawDelta, rollDelta);

	//---------- DEBUG ---------//
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, "HIGH SCROLL VALUE: " + FString::SanitizeFloat(_highScroll, 1) + "\nLOW SCROLL VALUE: " + FString::SanitizeFloat(_lowScroll, 1));
}

void AMainPlayerController::MoveForward (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->GetIsDead ())	//And the player is dead, don't do anything
			return;
	}

	if (value != .0f )
	{
		//Add movement in that direction
		GetCharacter ()->AddMovementInput (GetCharacter ()->GetActorForwardVector (), value);
	}
}

void AMainPlayerController::Strafe (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->GetIsDead())	//And the player is dead, don't do anything
			return;
	}
	
	if (value != .0f)
	{
		//Add movement in that direction
		GetCharacter ()->AddMovementInput (GetCharacter ()->GetActorRightVector (), value);
	}
}

void AMainPlayerController::VerticalStrafe (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->GetIsDead())	//And the player is dead, don't do anything
			return;
	}

	if (value != .0f)
	{
		GetCharacter()->AddMovementInput(GetCharacter()->GetActorUpVector(), value);
	}
}

void AMainPlayerController::Roll (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->GetIsDead ())	//And the player is dead, don't do anything
			return;
	}

	if (value != .0f)
	{
		//GetCharacter ()->AddControllerRollInput (value * GetWorld ()->DeltaTimeSeconds * 50.0f);
		rollDelta = value * _rollSpeed * GetWorld()->DeltaTimeSeconds;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Roll Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaRoll value = " + FString::SanitizeFloat(rollDelta, 2));
}

void AMainPlayerController::Pitch (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		//If player is using the mouse to click on spaceship UI, prevent rotation based on mouse axis value
		if (!_character->GetCanMove ()) return;
	}

	if (value != .0f)
	{
		//GetCharacter ()->AddControllerPitchInput (value * GetWorld ()->DeltaTimeSeconds * 10.0f);
		pitchDelta = value * _turnSpeed * GetWorld()->DeltaTimeSeconds;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Pitch Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaPitch value = " + FString::SanitizeFloat(pitchDelta, 2));
}

void AMainPlayerController::Yaw (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		//If player is using the mouse to click on spaceship UI, prevent rotation based on mouse axis value
		if (!_character->GetCanMove ()) return;
	}

	if (value != .0f)
	{
		//GetCharacter ()->AddControllerYawInput (value * GetWorld ()->DeltaTimeSeconds * 10.0f);
		yawDelta = value * _turnSpeed * GetWorld()->DeltaTimeSeconds;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Yaw Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaYaw value = " + FString::SanitizeFloat(yawDelta, 2));
}

void AMainPlayerController::UpdatePlayerRotation(float pitch, float yaw, float roll) 
{
	//If _character doesn't have a pointer, get one and wait a frame
	if (_character == nullptr) 
	{
		_character = Cast <AMainCharacterController>(GetCharacter());
		return;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("Can move Bool: %s"), _character->GetCanMove() ? TEXT("true") : TEXT("false")));

	//Make delta rotation in a Rotator and add it to the player delta rotation variable in MainCharacterController class
	FRotator newDeltaRotation = FRotator(pitch,yaw,roll);
	GetCharacter()->AddActorLocalRotation(newDeltaRotation, false, 0, ETeleportType::None);
	SetControlRotation (GetCharacter ()->GetActorRotation ());

	//Reset rotation values
	yawDelta = .0f;
	pitchDelta = .0f;
	rollDelta = .0f;
}

void AMainPlayerController::UpdateSpeed_Implementation (float value) 
{
	if (_character == nullptr || _UCharMoveComp == nullptr)
		return;

	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, "Current speed: " + FString::SanitizeFloat(_UCharMoveComp->MaxFlySpeed, 1));

	//If current speed is less or higher than max/min speed after last frame, set it to max/min
	if (_UCharMoveComp->MaxFlySpeed > _maxSpeed) { _UCharMoveComp->MaxFlySpeed = _maxSpeed; return; }
	else if (_UCharMoveComp->MaxFlySpeed < _minSpeed) { _UCharMoveComp->MaxFlySpeed = _minSpeed; return; }

	if (value != 0.0f) 
	{
		/*
		if (value > 0.0f)
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, "Mouse wheel UP! Value: " + FString::SanitizeFloat(value, 1));
		else if (value < 0.0f)
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, "Mouse wheel DOWN! Value: " + FString::SanitizeFloat(value, 1));
		*/

		if (_UCharMoveComp->MaxFlySpeed <= _maxSpeed && _UCharMoveComp->MaxFlySpeed >= _minSpeed) 
		{
			_UCharMoveComp->MaxFlySpeed += value * _deltaAcceleration * GetWorld()->DeltaTimeSeconds;
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, "Current speed: " + FString::SanitizeFloat(_UCharMoveComp->MaxFlySpeed, 1));
		}
	}
}

 bool AMainPlayerController::UpdateSpeed_Validate (float value)
{
	 return true;
}

void AMainPlayerController::RegisterPlayer_Implementation (const FString& playerName, int targetPlayerCount)
{
	//Register player to the game state class
	AMainGameState* gameState = Cast <AMainGameState> (GetWorld ()->GetGameState ());
	gameState->RegisterPlayer (this, playerName, targetPlayerCount);
}

bool AMainPlayerController::RegisterPlayer_Validate (const FString& playerName, int targetPlayerCount)
{
	return true;
}

void AMainPlayerController::SetupInputComponent ()
{
	Super::SetupInputComponent ();

	check (InputComponent);

	if (InputComponent != NULL)
	{
		//Set up movement bindings
		InputComponent->BindAxis ("MoveForward", this, &AMainPlayerController::MoveForward);
		InputComponent->BindAxis ("Strafe", this, &AMainPlayerController::Strafe);
		InputComponent->BindAxis ("Vertical_Strafe", this, &AMainPlayerController::VerticalStrafe);

		//Set up "look" bindings
		InputComponent->BindAxis ("Yaw", this, &AMainPlayerController::Yaw);
		InputComponent->BindAxis ("Pitch", this, &AMainPlayerController::Pitch);
		InputComponent->BindAxis ("Roll", this, &AMainPlayerController::Roll);
		InputComponent->BindAxis ("UpdateSpeed",this,&AMainPlayerController::UpdateSpeed);
	}
}