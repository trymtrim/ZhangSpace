// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainCharacterController.h"
#include "MainPlayerController.generated.h"

UCLASS()
class ZHANGSPACE_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY ()

public:
	AMainPlayerController();

	//Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetupInputComponent () override;

	void UpdatePlayerRotation (float pitch, float yaw, float roll);

	//Handles input for moving forward and backward
	UFUNCTION ()
	void MoveForward (float value);

	//Handles input for moving right and left
	UFUNCTION ()
	void Strafe (float value);

	UFUNCTION()
	void VerticalStrafe (float value);

	//Handles the roll feature of the spacecraft
	void Roll (float value);

	//Handles the pitching feature of the spacecraft
	void Pitch (float value);

	//Handles the yaw feature of the spacecraft
	void Yaw (float value);

	//Handles the change of speed
	UFUNCTION(Server, Reliable, WithValidation)
	void UpdateSpeed (float value);

	UFUNCTION (Server, Reliable, WithValidation)
	void RegisterPlayer (const FString& playerName, int targetPlayerCount);

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	//---------- MOVEMENT VALUES ----------//
	float _rollSpeed = 100.0f;				//Used to determine roll speed
	float const _minSpeed = 1500.0f;		//Used as the lowest possible speed when flying
	float const _maxSpeed = 10000.0f;		//Used as the default max speed
	float _deltaAcceleration = 3000.0f;

	//----------- ROTATION VALUES ----------//
	float pitchDelta = .0f;
	float yawDelta = .0f;
	float rollDelta = .0f;

	float _turnSpeed = 20.0f;				//Determines the rotation speed when using the mouse to rotate the ship based on delta values

	//Pointer reference to the character class and its CharacterMovementComponent
	AMainCharacterController* _character = nullptr;
	UCharacterMovementComponent* _UCharMoveComp = nullptr;
};
