// Copyright Team Monkey Business 2018.

#include "MainPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

void AMainPlayerController::MoveForward (float value)
{
	if (value != 0.0f)
	{
		//Add movement in that direction
		GetCharacter ()->AddMovementInput (GetCharacter ()->GetActorForwardVector (), value);
	}
}

void AMainPlayerController::MoveRight (float value)
{
	if (value != 0.0f)
	{
		//Add movement in that direction
		GetCharacter ()->AddMovementInput (GetCharacter ()->GetActorRightVector (), value);
	}
}

void AMainPlayerController::Roll (float value)
{
	//Debug
	//GEngine->AddOnScreenDebugMessage (-1, .01f, FColor::Yellow, "Roll Input Value = " + FString::SanitizeFloat (value, 2));

	if (value != .0f)
	{
		//Add new rotation value to current value
		FRotator newRotation = FRotator (.0f, .0f, value);
		FQuat quaternionRotation = FQuat (newRotation);	//Make a quaternion based of FRotator 
		GetCharacter ()->AddActorLocalRotation (quaternionRotation, false, 0, ETeleportType::None);	//Rotate around local axis with quat
	}
}

void AMainPlayerController::Pitch (float value)
{
	//Debug
	//GEngine->AddOnScreenDebugMessage (-1, .01f, FColor::Yellow, "Pitch Input Value = " + FString::SanitizeFloat (value, 2));

	if (value != .0f)
	{
		//Add new rotation value to current value
		//FRotator newRotation = FRotator (value * 20.0f * GetWorld ()->DeltaTimeSeconds, .0f, .0f);
		//FQuat quaternionRotation = FQuat (newRotation);	//Make a quaternion based of FRotator 
		//GetCharacter ()->AddActorLocalRotation (quaternionRotation, false, 0, ETeleportType::None);	//Rotate around local axis with quat
		
		GetCharacter ()->AddControllerPitchInput(value);
	}
}

void AMainPlayerController::Yaw (float value)
{
	//Debug
	//GEngine->AddOnScreenDebugMessage (-1, .01f, FColor::Yellow, "Yaw Input Value = " + FString::SanitizeFloat (value, 2));

	if (value != .0f)
	{
		//Add new rotation value to current value
		//FRotator newRotation = FRotator (.0f, value * 20.0f * GetWorld ()->DeltaTimeSeconds, .0f);
		//FQuat quaternionRotation = FQuat (newRotation);	//Make a quaternion based of FRotator 
		//GetCharacter ()->AddActorLocalRotation (quaternionRotation, false, 0, ETeleportType::None);	//Rotate around local axis with quat
		
		GetCharacter ()->AddControllerYawInput(value);
	}
}

void AMainPlayerController::SetupInputComponent ()
{
	Super::SetupInputComponent ();

	check (InputComponent);

	if (InputComponent != NULL)
	{
		//Set up movement bindings
		InputComponent->BindAxis ("MoveForward", this, &AMainPlayerController::MoveForward);
		InputComponent->BindAxis ("MoveRight", this, &AMainPlayerController::MoveRight);

		InputComponent->BindAxis ("Roll", this, &AMainPlayerController::Roll);

		//Set up "look" bindings
		//PlayerInputComponent->BindAxis("Yaw", this, &AMainPlayerController::AddControllerYawInput);
		InputComponent->BindAxis ("Yaw", this, &AMainPlayerController::Yaw);
		//PlayerInputComponent->BindAxis("Pitch", this, &AMainPlayerController::AddControllerPitchInput);
		InputComponent->BindAxis ("Pitch", this, &AMainPlayerController::Pitch);
	}
}
