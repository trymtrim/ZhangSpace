// Copyright Team Monkey Business 2018.

#include "MainPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "UnrealNetwork.h"
#include "Engine.h"

AMainPlayerController::AMainPlayerController()
{
	//Set this character to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


//Called when the game starts or when spawned
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Lock mouse to viewport
	if (GetWorld()->GetGameViewport() != nullptr)
	{
		GetWorld()->GetGameViewport()->SetMouseLockMode(EMouseLockMode::LockAlways);
		GetWorld()->GetGameViewport()->Viewport->LockMouseToViewport(true);
		
		//GEngine->GameViewport->Viewport->LockMouseToViewport(true); //Testing purposes for now
	}
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
	else if (_character == nullptr)
		_character = Cast <AMainCharacterController> (GetCharacter ());
}

void AMainPlayerController::MoveForward (float value)
{
	if (_character != nullptr)
	{
		if (_character->GetIsDead ())
			return;
	}

	if (value != .0f )
	{
		//Add movement in that direction

		//float speed = value * _moveSpeed * GetWorld()->DeltaTimeSeconds;
		GetCharacter ()->AddMovementInput (GetCharacter ()->GetActorForwardVector (), value);
	}
}

void AMainPlayerController::Strafe (float value)
{
	if (_character != nullptr)
	{
		if (_character->GetIsDead ())
			return;
	}
	
	if (value != .0f)
	{
		//Add movement in that direction
		//float speed = value * _strafeSpeed * GetWorld()->DeltaTimeSeconds;
		GetCharacter ()->AddMovementInput (GetCharacter ()->GetActorRightVector (), value);
	}
}

void AMainPlayerController::VerticalStrafe (float value)
{
	if (_character != nullptr)
	{
		if (!_character->GetIsDead ())
			return;
	}

	//TODO: Add vertical strafe, add bindaction to manipulate a bool if shift key is pressed
	if (value != .0f)
	{
		//float speed = value * _strafeSpeed * GetWorld()->DeltaTimeSeconds;
		GetCharacter()->AddMovementInput(GetCharacter()->GetActorUpVector(), value);
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

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("Can move Bool: %s"), _character->GetCanMove() ? TEXT("true") : TEXT("false")));

	//If player is using the mouse to click on spaceship UI, prevent rotation based on mouse axis value
	if (!_character->GetCanMove()) return;

	//Make delta rotation in a Rotator and add it to the player delta rotation variable in MainCharacterController class
	FRotator newDeltaRotation = FRotator(pitch,yaw,roll);
	GetCharacter()->AddActorLocalRotation(newDeltaRotation, false, 0, ETeleportType::None);

	//Update player rotation on server to match client rotation
	_character->_playerRotation = GetCharacter()->GetActorRotation();

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
		InputComponent->BindAxis ("Vertical_Strafe", this, &AMainPlayerController::VerticalStrafe);

		//Set up "look" bindings
		InputComponent->BindAxis ("Yaw", this, &AMainPlayerController::Yaw);
		InputComponent->BindAxis ("Pitch", this, &AMainPlayerController::Pitch);
		InputComponent->BindAxis ("Roll", this, &AMainPlayerController::Roll);
	}
}
