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
	UFUNCTION()
	void MoveForward (float value);

	//Handles input for moving right and left
	UFUNCTION()
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
	void ChargeCruiseSpeed ();
	UFUNCTION (Server, Reliable, WithValidation)
	void StopChargeCruiseSpeed ();

	//Let's the player brake the spaceship to a full stop
	UFUNCTION(Server, Reliable, WithValidation)
	void Brake();

	UFUNCTION (Server, Reliable, WithValidation)
	void StopBrake();

	//Handles the change of speed
	UFUNCTION(Server, Reliable, WithValidation)
	void IncreaseSpeed (float value);

	UFUNCTION (Server, Reliable, WithValidation)
	void RegisterPlayer (const FString& playerName, int targetPlayerCount);

	UPROPERTY (Replicated, BlueprintReadOnly) bool _cruiseSpeed = false;	//Determines if player has entered cruise speed or not
	UPROPERTY (Replicated, BlueprintReadOnly) float _chargeRatio = .0f;		//Accessed through blueprint for UI
	UPROPERTY (Replicated, BlueprintReadOnly) float _cooldownRatio = .0f;	//Blueprint exposed

	//----------- ROTATION VALUES ----------//
	UPROPERTY(BlueprintReadOnly) float pitchDelta = .0f;
	UPROPERTY(BlueprintReadOnly) float yawDelta = .0f;
	float rollDelta = .0f;

	UPROPERTY (BlueprintReadOnly) float _maxDeltaValue = .5f;	//Used to clamp cruise speed delta values
	UPROPERTY (BlueprintReadOnly) float _minDeltaValue = .1f;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	//---------- MOVEMENT VALUES ----------//
	float _rollSpeed = 80.0f;					//Used to determine roll speed
	float const MINIMUM_SPEED = 8000.0f;		//Used as the lowest possible speed when flying
	float const MAXIMUM_SPEED = 14000.0f;
	float const MINIMUM_ACCEL = 4000.0f;
	float const MAXIMUM_ACCEL = 16000.0f;
	UPROPERTY (Replicated) float _maxSpeed = 5000.0f;			//Used as the default max speed
	UPROPERTY (Replicated) float _acceleration = 4000.0f;		//The rate at which the speed increases when scrolling, is multiplied with scroll axis value (Not in cruise speed)
	UPROPERTY (Replicated) bool _braking = false;				//Used to braking the spaceship manually
	float _sensitivityScaler = 20.0f;							//Used to scale sensitivity with mouse input
	float _turnSpeed = 20.0f;					//Determines the rotation speed when using the mouse to rotate the ship based on delta values, when not in cruise speed
	float _defaultAcceleration = 2000.0f;		//Default acceleration in general settings in movementcomp when not in cruise speedw
	float _cruiseSpeedAcceleration = 10000.0f;	//Acceleration when using cruise speed
	
	//---------- CRUISE SPEED CHARGE VALUES ----------//
	float _currentCharge = .0f;
	float _CSCooldown = .0f;
	float _chargeTime = 3.0f;
	float const CS_CD = 5.0f;
	float const DEFAULT_CHARGE_TIME = 3.0f;
	float const MINIMUM_CHARGE_TIME = 1.5f;

	bool _charge = false;				//Used to keep track of cruise speed charge up

	//Local mobility stat, updated through the server
	int  _currentMobilityStat = 1;

	//Pointer reference to the character class and its CharacterMovementComponent
	AMainCharacterController* _character = nullptr;
	UCharacterMovementComponent* _UCharMoveComp = nullptr;

	//Private local scope functions
	void UpdateSpeedAndAcceleration(int mobilityStat);
	void SetCruiseValues();
	void SetDefaultSpeedAndAcceleration();
};
