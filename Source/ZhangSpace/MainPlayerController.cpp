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

	//Channeling beam
	//_character->GetChannelingBeam ();
	//Boost
	//_character->GetBoost ();
}

//Called every frame
void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Always look for a reference to the player and assign it
	if (_character == nullptr) 
	{
		_character = Cast <AMainCharacterController>(GetCharacter());

		if (_UCharMoveComp == nullptr && _character != nullptr)
			_UCharMoveComp = _character->GetCharacterMovement();
		return;
	}

	if (!GetWorld ()->IsServer ())
		UpdatePlayerRotation (pitchDelta, yawDelta, rollDelta);

	if (_character != nullptr && _UCharMoveComp != nullptr)
	{
		if (!_character->GetIsDead ())
		{
			//--------------- ENTERING CRUISE SPEED ---------------//
			//Normally the movement is done when pressing down a button, when in cruise speed we want constant movement forward, so 1.0f as scale
			if (_cruiseSpeed)
			{
				_character->AddMovementInput(GetCharacter()->GetActorForwardVector(), 1.0f);
			}
		}

		//--------------- UPDATE CRUISE SPEED VALUES ---------------//
		//Update the speed and acceleration depending on whether or not we are in cruise speed
		if (_cruiseSpeed)
		{
			//Update max speed and acceleration accordingly, current mobility determines max speed and acceleration, which again is factored
			SetCruiseValues ();
		}
		else
			SetDefaultSpeedAndAcceleration ();	//Reset values to current values based on mobilitypower

		//---------- SERVER ----------//
		if (GetWorld ()->IsServer ())
		{
			//---------- UPDATE MOBILITYPOWER IF CHANGED ----------//
			if (_currentMobilityStat != _character->GetMobilityPower()) 
			{
				//To update mobility power (values = 1-10):
				_currentMobilityStat = _character->GetMobilityPower();
				
				UpdateSpeedAndAcceleration(_currentMobilityStat);
			}

			//Update charge and cd ratio on server
			_chargeRatio = _currentCharge / _chargeTime;
			_cooldownRatio = _CSCooldown / CS_CD;
			_boost = _character->GetBoost ();

		}

		//---------- KEEP TRACK OF CRUISE SPEED CHARGING ----------//

		if (_charge && !_cruiseSpeed && !_character->GetChannelingBeam ())	//If charging and not in cruise speed
		{
			if (_currentCharge <= _chargeTime && _CSCooldown == .0f)
			{	
				_currentCharge += GetWorld()->DeltaTimeSeconds;	//Increase charge cooldown
			}
			else if (_currentCharge > _chargeTime)
			{
				_currentCharge = _chargeTime;	//Set max value so it doesn't go above max
				_charge = false;
				_cruiseSpeed = true;	//Enter cruise speed when fully charged
			}
		}
		else if (!_charge && !_cruiseSpeed)		//If we release charge button before entering cruise speed
		{
			if (_currentCharge > .0f)
				_currentCharge -= GetWorld ()->DeltaTimeSeconds;	//Decrease charge cooldown
			else if (_currentCharge < .0f)
				_currentCharge = .0f;				//If less than zero, clamp to zero 
		}

		//---------- KEEP TRACK OF COOLDOWN FOR CRUISE SPEED ----------//
		if (!_cruiseSpeed && _CSCooldown > .0f)	//We have exited cruise speed and cooldown is set
		{
			_CSCooldown -= GetWorld()->DeltaTimeSeconds;	//Decrease cooldown timer
		}
		else if (!_cruiseSpeed && _CSCooldown < .0f)	//We have exited cruise speed and cooldown is done
		{
			_CSCooldown = .0f; //Reset cooldown
		}


		//Apply braking if we're not in cruise speed
		if (_braking && !_cruiseSpeed) 
		{	
			
			//_UCharMoveComp->ApplyVelocityBraking(GetWorld()->DeltaTimeSeconds, 5.0f, _acceleration);
			_UCharMoveComp->CalcVelocity(GetWorld()->DeltaTimeSeconds, .5f, false, .0f);
			//GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Braking: true");
		}
		if (_character->GetChannelingBeam ()) //When charging up hyper beam, restrict movement
		{
			_braking = true;
			_cruiseSpeed = false;
		}
		else _braking = false;	//If not channeling beam and braking is true, don't brake anymore
		//else GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Braking: false");
		
		//GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Charge value: " + FString::SanitizeFloat (_currentCharge));
		//GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Cruise speed CD value: " + FString::SanitizeFloat (_CSCooldown));

		//if (_boost) GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Boosting is true");
		//else GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Boosting is false");
	}

	//---------- DEBUG MESSAGES ON SCREEN ----------//
	//GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Charge ratio: " + FString::SanitizeFloat (_chargeRatio));
	//GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Cooldown ratio: " + FString::SanitizeFloat (_cooldownRatio));
}

void AMainPlayerController::MoveForward (float value)
{
	if (_character != nullptr)			//If we have a reference to the character pointer
	{
		if (_character->GetIsDead ())	//And the player is dead, don't do anything
			return;
	}

	if (_cruiseSpeed || _braking) return;		//If in cruise speed or braking, do not update movement based on input

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
	
	if (_cruiseSpeed || _braking) return;		//If in cruise speed or braking, do not update movement based on input

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
	if (_cruiseSpeed || _braking) return; //If in cruise speed or braking, dont strafe up or down

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

	if (value != .0f)
	{
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
		if (_cruiseSpeed) //Lower the pitch sensitivity and make it continously
		{
			pitchDelta += value / _sensitivityScaler;

			if (pitchDelta >= _maxDeltaValue)
				pitchDelta = _maxDeltaValue;
			else if (pitchDelta <= -_maxDeltaValue)
				pitchDelta = -_maxDeltaValue;
		}
		else if (_character->GetChannelingBeam ()) //When charging hyper beam, restrict sensitivity when pitching
		{
			pitchDelta = (value * _turnSpeed * GetWorld ()->DeltaTimeSeconds) / 5.0f;;
		}
		else //Make pitch like FPS controls
			pitchDelta = value * _turnSpeed * GetWorld()->DeltaTimeSeconds;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Pitch Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaPitch value = " + FString::SanitizeFloat(pitchDelta, 2));
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
		else if (_character->GetChannelingBeam ()) //When charging hyper beam
		{
			//Decrease yaw sensitivity
			yawDelta = (value * _turnSpeed * GetWorld ()->DeltaTimeSeconds) / 5.0f;
		}
		else
		{
			//Do normal delta calculation based on turnSpeed when not in cruise speed
			yawDelta = value * _turnSpeed * GetWorld()->DeltaTimeSeconds;
		}
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Yaw Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaYaw value = " + FString::SanitizeFloat(yawDelta, 2));
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

 void AMainPlayerController::Brake_Implementation() 
 {
	 if (_character != nullptr)			//If we have a reference to the character pointer
	 {
		 if (_character->GetIsDead ())	//And the player is dead, don't do anything
			 return;
	 }

	 if (_cruiseSpeed) _cruiseSpeed = false;

	 _braking = true;
 }

 bool AMainPlayerController::Brake_Validate() 
 {
	 return true;
 }

 void AMainPlayerController::StopBrake_Implementation ()
 {
	 _braking = false;
 }

 bool AMainPlayerController::StopBrake_Validate ()
 {
	 return true;
 }

 //Called when Cruise Speed button is pressed
 void AMainPlayerController::ChargeCruiseSpeed_Implementation ()
 {
	 if (_character != nullptr)			//If we have a reference to the character pointer
	 {
		 if (_character->GetIsDead ())	//And the player is dead, don't do anything
			 return;
	 }

	 if (_cruiseSpeed) 
	 { 
		 _cruiseSpeed = false; 			//Go out of cruise speed if we release the button while in cruisespeed
		 _currentCharge = .0f;				//Reset charge value if we exit cruise speed, so that progress bar stays green

		 //Set cooldown
		 if (_CSCooldown == .0f)
			 _CSCooldown = CS_CD;
	 
	 }
	 else _charge = true;						//Continue charging
 }

 bool AMainPlayerController::ChargeCruiseSpeed_Validate ()
 {
	 return true;
 }

 void AMainPlayerController::StopChargeCruiseSpeed_Implementation ()
 {
	 if (_character != nullptr)			//If we have a reference to the character pointer
	 {
		 if (_character->GetIsDead ())	//And the player is dead, don't do anything
			 return;
	 }

	 if (_charge) _charge = false;	//Stop charging if we release the button too early
 }

 bool AMainPlayerController::StopChargeCruiseSpeed_Validate ()
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
	//---------- CALCULATE DIFFERENCE AND STEP FOR EACH LEVEL ----------//
	//Increase acceleration and speed thresholds based on mobility stat and difference between each step. Linear behaviour
	float difference = (MAXIMUM_SPEED - MINIMUM_SPEED) / 8;		//divided by 8 since first and last mobility level are preset to min/max constants
	float chargeStep = (DEFAULT_CHARGE_TIME - MINIMUM_CHARGE_TIME) / 8;
	float accelerationStep = (MAXIMUM_ACCEL - MINIMUM_ACCEL) / 8;

	switch (mobilityPower)
	{
		case 1: { _maxSpeed = MINIMUM_SPEED; _acceleration = MINIMUM_ACCEL; _chargeTime = DEFAULT_CHARGE_TIME; break; }
		case 2: { _maxSpeed += difference; _acceleration += accelerationStep; _chargeTime -= chargeStep; break; }
		case 3: { _maxSpeed += difference; _acceleration += accelerationStep; _chargeTime -= chargeStep; break; }
		case 4: { _maxSpeed += difference; _acceleration += accelerationStep; _chargeTime -= chargeStep; break; }
		case 5: { _maxSpeed += difference; _acceleration += accelerationStep; _chargeTime -= chargeStep; break; }
		case 6: { _maxSpeed += difference; _acceleration += accelerationStep; _chargeTime -= chargeStep; break; }
		case 7: { _maxSpeed += difference; _acceleration += accelerationStep; _chargeTime -= chargeStep; break; }
		case 8: { _maxSpeed += difference; _acceleration += accelerationStep; _chargeTime -= chargeStep; break; }
		case 9: { _maxSpeed += difference; _acceleration += accelerationStep; _chargeTime -= chargeStep; break; }
		case 10: { _maxSpeed = MAXIMUM_SPEED; _acceleration = MAXIMUM_ACCEL; _chargeTime = MINIMUM_CHARGE_TIME; break; }
		default: { _maxSpeed = MINIMUM_SPEED; _acceleration = MINIMUM_ACCEL; _chargeTime = DEFAULT_CHARGE_TIME; break; }
	}
}

void AMainPlayerController::SetCruiseValues() 
{
	//Update max speed and acceleration accordingly, current mobility determines max speed and acceleration, which again is factored
	_UCharMoveComp->MaxFlySpeed = _maxSpeed * 2.5f;
	_UCharMoveComp->MaxAcceleration = _acceleration * 2.0f;
}

void AMainPlayerController::SetDefaultSpeedAndAcceleration () 
{
	//If the boost ability is used, update speed and acceleration to ludachris values instead...
	if (_boost)
	{
		_UCharMoveComp->MaxFlySpeed = _maxSpeed * 5.0f;
		_UCharMoveComp->MaxAcceleration = _acceleration * 5.0f;
		//GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Boost in running");
		return;	//Dont update speed to default if boost is used and not in cruise speed
	}
	//else GEngine->AddOnScreenDebugMessage (-1, .005f, FColor::Yellow, "Boost in not running");


	if (_UCharMoveComp->MaxFlySpeed != _maxSpeed && _UCharMoveComp->MaxAcceleration != _acceleration)
	{
		_UCharMoveComp->MaxFlySpeed = _maxSpeed;
		_UCharMoveComp->MaxAcceleration = _acceleration;
		//GEngine->AddOnScreenDebugMessage (-1, 2.0f, FColor::Red, "Set to default values");
	}
}

void AMainPlayerController::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME (AMainPlayerController, _cruiseSpeed);
	DOREPLIFETIME (AMainPlayerController, _maxSpeed);
	DOREPLIFETIME (AMainPlayerController, _acceleration);
	DOREPLIFETIME (AMainPlayerController, _chargeRatio);
	DOREPLIFETIME (AMainPlayerController, _cooldownRatio);
	DOREPLIFETIME (AMainPlayerController, _braking);
	DOREPLIFETIME (AMainPlayerController, _boost);
}

void AMainPlayerController::SetupInputComponent ()
{
	Super::SetupInputComponent ();

	check (InputComponent);

	if (InputComponent != NULL)
	{
		//Set up action binding
		InputComponent->BindAction ("CruiseSpeed", IE_Pressed, this, &AMainPlayerController::ChargeCruiseSpeed);
		InputComponent->BindAction ("CruiseSpeed", IE_Released, this, &AMainPlayerController::StopChargeCruiseSpeed);

		InputComponent->BindAction ("Brake", IE_Pressed, this, &AMainPlayerController::Brake);
		InputComponent->BindAction ("Brake", IE_Released, this, &AMainPlayerController::StopBrake);

		//Set up movement bindings
		InputComponent->BindAxis ("MoveForward", this, &AMainPlayerController::MoveForward);
		InputComponent->BindAxis ("Strafe", this, &AMainPlayerController::Strafe);
		InputComponent->BindAxis ("Vertical_Strafe", this, &AMainPlayerController::VerticalStrafe);

		//Set up "look" bindings
		InputComponent->BindAxis ("Yaw", this, &AMainPlayerController::Yaw);
		InputComponent->BindAxis ("Pitch", this, &AMainPlayerController::Pitch);
		InputComponent->BindAxis ("Roll", this, &AMainPlayerController::Roll);
	}
}