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

	//This is preventing gimbal lock
	PlayerCameraManager->ViewPitchMax = 359.998993f;
	PlayerCameraManager->ViewPitchMin = 0.0f;
	PlayerCameraManager->ViewYawMax = 359.998993f;
	PlayerCameraManager->ViewRollMax = 359.998993f;
	PlayerCameraManager->ViewRollMin = 0.0f;

	//Register player for game state
	if (!GetWorld ()->IsServer ())
		RegisterPlayer (ConfigManager::GetConfig ("Player_Name"), Cast <USettingsManager> (GetWorld ()->GetGameInstance ())->GetTargetPlayerCount ());

}

//Called every frame
void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Always look for a reference to the player 
	if (_character == nullptr) 
	{
		_character = Cast <AMainCharacterController>(GetCharacter());

		if (_UCharMoveComp == nullptr && _character != nullptr)
			_UCharMoveComp = _character->GetCharacterMovement();
	}

	if (!GetWorld ()->IsServer ())
		UpdatePlayerRotation (pitchDelta, yawDelta, rollDelta);

	//Normally the movement is done when pressing down a button, when in cruise speed we want constant movement forward
	if (_cruiseSpeed)
	{
		_character->AddMovementInput(GetCharacter()->GetActorForwardVector(), 1.0f);
	}

	if (_character != nullptr)
	{
		if (_currentMobilityStat != _character->GetMobilityPower()) 
		{
			//To update mobility power (values = 1-10):
			_currentMobilityStat = _character->GetMobilityPower();
		}

		UpdateSpeedAndAcceleration(_currentMobilityStat);

		GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Speed: " + FString::SanitizeFloat (_UCharMoveComp->MaxFlySpeed));
	}

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

	if (_cruiseSpeed) return;		//If in cruise speed, do not update movement based on input

	if (value != .0f )
	{
		//Add movement in that direction based on value
		_character->AddMovementInput (_character->GetActorForwardVector (), value);
	}
}

void AMainPlayerController::Strafe (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->GetIsDead())	//And the player is dead, don't do anything
			return;
	}
	
	if (_cruiseSpeed) return;		//If in cruise speed, do not update movement based on input

	if (value != .0f)
	{
		//Add movement in that direction
		_character->AddMovementInput (_character->GetActorRightVector (), value);
	}
}

void AMainPlayerController::VerticalStrafe (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->GetIsDead())	//And the player is dead, don't do anything
			return;
	}
	if (_cruiseSpeed) return;

	if (value != .0f)
	{
		_character->AddMovementInput(_character->GetActorUpVector(), value);
	}
}

void AMainPlayerController::Roll (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->isSpectating)
			_character->SpectateRotateBP ("Roll", value);

		if (_character->GetIsDead ())	//And the player is dead, don't do anything
			return;
	}

	if (_cruiseSpeed) 
	{ 
		//rollDelta = FMath::FInterpTo(rollDelta, .0f, GetWorld()->DeltaTimeSeconds, 2.0f); 
		return;
	}

	if (value != .0f)
	{
		//GetCharacter ()->AddControllerRollInput (value * GetWorld ()->DeltaTimeSeconds * 50.0f);

		rollDelta = value * _rollSpeed * GetWorld()->DeltaTimeSeconds;
	}
	
	if (rollDelta > .0f || rollDelta < .0f)
		rollDelta = FMath::FInterpTo(rollDelta, .0f, GetWorld()->DeltaTimeSeconds, 2.0f);

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Roll Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaRoll value = " + FString::SanitizeFloat(rollDelta, 2));
}

void AMainPlayerController::Pitch (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->isSpectating)
			_character->SpectateRotateBP ("Pitch", value);

		//If player is using the mouse to click on spaceship UI, prevent rotation based on mouse axis value
		if (!_character->GetCanMove ()) return;
	}

	if (value != .0f)
	{
		if (_cruiseSpeed) 
		{
			pitchDelta += value / _sensitivityScaler;

			if (pitchDelta >= _maxDeltaValue)
				pitchDelta = _maxDeltaValue;
			else if (pitchDelta <= -_maxDeltaValue)
				pitchDelta = -_maxDeltaValue;
		}
		else
			pitchDelta = value * _turnSpeed * GetWorld()->DeltaTimeSeconds;
	}

	//Debug
	GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Pitch Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaPitch value = " + FString::SanitizeFloat(pitchDelta, 2));
}

void AMainPlayerController::Yaw (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->isSpectating)
			_character->SpectateRotateBP ("Yaw", value);

		//If player is using the mouse to click on spaceship UI, prevent rotation based on mouse axis value
		if (!_character->GetCanMove ()) return;
	}

	if (value != .0f)
	{

		//GetCharacter ()->AddControllerYawInput (value * GetWorld ()->DeltaTimeSeconds * 10.0f);
		if (_cruiseSpeed)
		{
			//Add value to yaw and roll constantly
			yawDelta += value / (_sensitivityScaler);
			rollDelta += value;				

			//If the yaw and roll delta values are higher or lower than the defined sensitivity, clamp value
			if (yawDelta >= _maxDeltaValue)
				yawDelta = _maxDeltaValue;
			else if (yawDelta <= -_maxDeltaValue)
				yawDelta = -_maxDeltaValue;

			if (rollDelta >= _maxDeltaValue / 2.0f)
				rollDelta = _maxDeltaValue / 2.0f;
			else if (rollDelta <= -(_maxDeltaValue / 2.0f))
				rollDelta = -(_maxDeltaValue / 2.0f);
		}
		else
		{
			//Do normal delta calculation based on turnSpeed when not in cruise speed
			yawDelta = value * _turnSpeed * GetWorld()->DeltaTimeSeconds;
		}
	}

	//Debug
	GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Yaw Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaYaw value = " + FString::SanitizeFloat(yawDelta, 2));
}

void AMainPlayerController::UpdatePlayerRotation(float pitch, float yaw, float roll) 
{
	//Deadzone for registering input when in cruise speed
	if (_cruiseSpeed)
	{
		if (FMath::Abs (pitch) < _minDeltaValue && FMath::Abs (yaw) < _minDeltaValue)
			return;
	}

	//If _character doesn't have a pointer, get one and wait a frame
	if (_character == nullptr) 
	{
		_character = Cast <AMainCharacterController>(GetCharacter());
		return;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("Can move Bool: %s"), _character->GetCanMove() ? TEXT("true") : TEXT("false")));

	//Make delta rotation in a Rotator and add it to local rotation on client side, then update the charactercontroller on server side
	FRotator newDeltaRotation = FRotator(pitch,yaw,roll);
	_character->AddActorLocalRotation(newDeltaRotation, false, 0, ETeleportType::None);

	//Update the controller server side
	SetControlRotation (_character->GetActorRotation ());

	if (_cruiseSpeed) return; //Do not reset values if in cruise speed

	//Reset rotation values, this simulates fps controls when not in cruise speed
	yawDelta = .0f;
	pitchDelta = .0f;
	//rollDelta = .0f;
}

void AMainPlayerController::UpdateAcceleration_Implementation(float value) 
{
	if (_UCharMoveComp != nullptr) 
	{
		_UCharMoveComp->MaxAcceleration = value;
		ClientUpdateAcceleration(value);
	}
}

bool AMainPlayerController::UpdateAcceleration_Validate(float value)
{
	return true;
}

void AMainPlayerController::ClientUpdateAcceleration_Implementation(float value)
{
	if (_UCharMoveComp != nullptr) {
		_UCharMoveComp->MaxAcceleration = value;
	}
}

void AMainPlayerController::IncreaseSpeed_Implementation (float value)
{
	if (_character == nullptr || _UCharMoveComp == nullptr)
		return;

	//GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Blue, "Current max speed: " + FString::SanitizeFloat(_UCharMoveComp->MaxFlySpeed, 1));
	//GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Blue, "Current max acceleration: " + FString::SanitizeFloat(_UCharMoveComp->MaxAcceleration, 1));


	//If we have entered cruisespeed, set the speed accordingly
	if (_cruiseSpeed)
	{
		_UCharMoveComp->MaxFlySpeed = _maxSpeed * 5.0f;
		UpdateAcceleration(_UCharMoveComp->MaxAcceleration * 5);
		return;
	}
	else
	{
		_UCharMoveComp->MaxFlySpeed = _maxSpeed;
		UpdateAcceleration(_UCharMoveComp->MaxAcceleration / 5);
		return;
	}

	//If current speed is less or higher than max/min speed after last frame, set it to max/min, preventing infintiely increase/decrease of speed when scrolling
	if (_UCharMoveComp->MaxFlySpeed > _maxSpeed && !_cruiseSpeed) { _UCharMoveComp->MaxFlySpeed = _maxSpeed; return; }
	else if (_UCharMoveComp->MaxFlySpeed < _minSpeed && !_cruiseSpeed) { _UCharMoveComp->MaxFlySpeed = _minSpeed; return; }

	if (value != 0.0f) 
	{
		/*
		if (value > 0.0f)
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, "Mouse wheel UP! Value: " + FString::SanitizeFloat(value, 1));
		else if (value < 0.0f)
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, "Mouse wheel DOWN! Value: " + FString::SanitizeFloat(value, 1));
		*/

		//Update speed based on input
		if (_UCharMoveComp->MaxFlySpeed <= _maxSpeed && _UCharMoveComp->MaxFlySpeed >= _minSpeed) 
		{
			float deltaAcceleration = value * _acceleration * GetWorld()->DeltaTimeSeconds;
			_UCharMoveComp->MaxFlySpeed += deltaAcceleration;
		}
	}
}

 bool AMainPlayerController::IncreaseSpeed_Validate (float value)
{
	 return true;
}

 //Called when Cruise Speed button is pressed once
 void AMainPlayerController::CruiseSpeed_Implementation ()
 {
	 _cruiseSpeed = !_cruiseSpeed;

	 if (_cruiseSpeed)
	 { 
		 //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Cruise speed: true");
		  //IncreaseSpeed_Implementation(1.0f);	//Call the update speed implementation which normally is called when scrolling mouse wheel, pass 1 as argument to simulate button press
	 }
	 //else GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Cruise speed: false");
 }

 bool AMainPlayerController::CruiseSpeed_Validate ()
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

void AMainPlayerController::UpdateSpeedAndAcceleration(int mobilityPower) 
{
	/*
		_minSpeed = 1500f
		_maxSpeed = 10000f
		default acceleration = 2000f
		acceleration is very fast at 10000f
	
	*/

	//Increase acceleration and speed thresholds based on mobility stat
	switch (mobilityPower)
	{
		case 1: { _maxSpeed = 5000.0f; _acceleration = 4000.0f; break; }
		case 2: { _maxSpeed = 6000.0f; _acceleration = 6000.0f; break; }
		case 3: { _maxSpeed = 7000.0f; _acceleration = 8000.0f; break; }
		case 4: { _maxSpeed = 8000.0f; _acceleration = 10000.0f; break; }
		case 5: { _maxSpeed = 9000.0f; _acceleration = 12000.0f; break; }
		case 6: { _maxSpeed = 10000.0f; _acceleration = 14000.0f; break; }
		case 7: { _maxSpeed = 11000.0f; _acceleration = 16000.0f; break; }
		case 8: { _maxSpeed = 12000.0f; _acceleration = 18000.0f; break; }
		case 9: { _maxSpeed = 13000.0f; _acceleration = 1900.0f; break; }
		case 10: { _maxSpeed = 14000.0f; _acceleration = 20000.0f; break; }
		default: { _maxSpeed = 5000.0f; _acceleration = 4000.0f;  break; }
	}

	if (_cruiseSpeed)
	{
		_UCharMoveComp->MaxFlySpeed = _maxSpeed * 2.5f;
		_UCharMoveComp->MaxAcceleration = _acceleration * 5.0f;
	}
	else
	{
		_UCharMoveComp->MaxFlySpeed = _maxSpeed;
		_UCharMoveComp->MaxAcceleration = _acceleration;

	}
}


void AMainPlayerController::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMainPlayerController, _cruiseSpeed);
}

void AMainPlayerController::SetupInputComponent ()
{
	Super::SetupInputComponent ();

	check (InputComponent);

	if (InputComponent != NULL)
	{
		//Set up action binding
		InputComponent->BindAction("CruiseSpeed", IE_Pressed, this, &AMainPlayerController::CruiseSpeed);

		//Set up movement bindings
		InputComponent->BindAxis ("MoveForward", this, &AMainPlayerController::MoveForward);
		InputComponent->BindAxis ("Strafe", this, &AMainPlayerController::Strafe);
		InputComponent->BindAxis ("Vertical_Strafe", this, &AMainPlayerController::VerticalStrafe);

		//Set up "look" bindings
		InputComponent->BindAxis ("Yaw", this, &AMainPlayerController::Yaw);
		InputComponent->BindAxis ("Pitch", this, &AMainPlayerController::Pitch);
		InputComponent->BindAxis ("Roll", this, &AMainPlayerController::Roll);
		//InputComponent->BindAxis ("UpdateSpeed",this,&AMainPlayerController::IncreaseSpeed);
	}
}