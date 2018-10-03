// Copyright Team Monkey Business 2018.

#include "MainPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "UnrealNetwork.h"

AMainPlayerController::AMainPlayerController()
{
	//Set this character to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called every frame
void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->IsServer())
	{
		UpdateRotation(pitchDelta, yawDelta, rollDelta);

		//Debug
		//if (_character != nullptr)
			//GEngine->AddOnScreenDebugMessage (-1, .01f, FColor::Yellow, "_character is not nullptr");
		
	}
}

void AMainPlayerController::MoveForward (float value)
{
	if (value != 0.0f )
	{
		//Add movement in that direction
		GetCharacter ()->AddMovementInput (GetCharacter ()->GetActorForwardVector (), value);
	}
}

void AMainPlayerController::Strafe (float value)
{
	if (value != 0.0f)
	{
		//Add movement in that direction
		GetCharacter ()->AddMovementInput (GetCharacter ()->GetActorRightVector (), value);
	}
}

void AMainPlayerController::Roll_Implementation (float value)
{

	if (value != .0f)
	{
		rollDelta = value * _rollSpeed * GetWorld()->DeltaTimeSeconds;
	}
	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Roll Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaRoll value = " + FString::SanitizeFloat(rollDelta, 2));
}

bool AMainPlayerController::Roll_Validate(float value) 
{
	return true;
}

void AMainPlayerController::Pitch_Implementation (float value)
{
	if (value != .0f)
	{
		pitchDelta = value * _turnSpeed * GetWorld()->DeltaTimeSeconds;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Pitch Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaPitch value = " + FString::SanitizeFloat(pitchDelta, 2));
}

bool AMainPlayerController::Pitch_Validate(float value) 
{
	return true;
}

void AMainPlayerController::Yaw_Implementation (float value)
{
	if (value != .0f)
	{
		yawDelta = value * _turnSpeed * GetWorld()->DeltaTimeSeconds;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, .005f, FColor::Yellow, "Yaw Input Value = " + FString::SanitizeFloat(value, 2) + ", deltaYaw value = " + FString::SanitizeFloat(yawDelta, 2));
}

bool AMainPlayerController::Yaw_Validate(float value) 
{
	return true;
}

void AMainPlayerController::UpdateRotation(float pitch, float yaw, float roll) 
{
	//If _character doesn't have a pointer, get one and return 1 frame
	if (_character == nullptr) 
	{
		_character = Cast <AMainCharacterController>(GetCharacter());
		return;
	}
	
	//Make delta rotation in a Rotator and add it to the player delta rotation variable in MainCharacterController class
	FRotator newDeltaRotation = FRotator(pitch,yaw,roll);
	_character->_playerDeltaRotation = newDeltaRotation;

	//Reset rotation values
	yawDelta = .0f;
	pitchDelta = .0f;
	rollDelta = .0f;
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

		//Set up "look" bindings
		InputComponent->BindAxis ("Yaw", this, &AMainPlayerController::Yaw);
		InputComponent->BindAxis ("Pitch", this, &AMainPlayerController::Pitch);
		InputComponent->BindAxis ("Roll", this, &AMainPlayerController::Roll);
	}
}
