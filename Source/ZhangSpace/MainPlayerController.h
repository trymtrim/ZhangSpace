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

	UFUNCTION(Server, Reliable, WithValidation)
	void UpdateAcceleration(float value);
	UFUNCTION(Client, Reliable)
	void ClientUpdateAcceleration(float value);

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

	//Cruise speed
	UFUNCTION(Server, Reliable, WithValidation)
	void CruiseSpeed();

	//Handles the change of speed
	UFUNCTION(Server, Reliable, WithValidation)
	void UpdateSpeed (float value);

	UFUNCTION (Server, Reliable, WithValidation)
	void RegisterPlayer (const FString& playerName, int targetPlayerCount);

	UPROPERTY(Replicated, BlueprintReadOnly) bool _cruiseSpeed = false;				//Determines if player has entered cruise speed or not

	//----------- ROTATION VALUES ----------//
	UPROPERTY(BlueprintReadOnly) float pitchDelta = .0f;
	UPROPERTY(BlueprintReadOnly) float yawDelta = .0f;
	float rollDelta = .0f;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	//---------- MOVEMENT VALUES ----------//
	float _rollSpeed = 80.0f;				//Used to determine roll speed
	float const _minSpeed = 1500.0f;		//Used as the lowest possible speed when flying
	float const _maxSpeed = 10000.0f;		//Used as the default max speed
	float _acceleration = 3000.0f;			//The rate at which the speed increases, is multiplied with scroll axis value


	float _turnSpeed = 20.0f;				//Determines the rotation speed when using the mouse to rotate the ship based on delta values

	//Pointer reference to the character class and its CharacterMovementComponent
	AMainCharacterController* _character = nullptr;
	UCharacterMovementComponent* _UCharMoveComp = nullptr;
};
