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

	void UpdateRotation(float pitch, float yaw, float roll);
	
	//Handles input for moving forward and backward
	UFUNCTION ()
	void MoveForward (float value);

	//Handles input for moving right and left
	UFUNCTION ()
	void Strafe (float value);

	UFUNCTION()
	void VerticalStrafe (float value);

	//Handles the roll feature of the spacecraft
	UFUNCTION(Server, Reliable, WithValidation)
	void Roll (float value);
	virtual void Roll_Implementation(float value);
	virtual bool Roll_Validate(float value);

	//Handles the pitching feature of the spacecraft
	UFUNCTION(Server, Reliable, WithValidation)
	void Pitch (float value);
	virtual void Pitch_Implementation(float value);
	virtual bool Pitch_Validate(float value);

	//Handles the yaw feature of the spacecraft
	UFUNCTION(Server, Reliable, WithValidation)
	void Yaw (float value);
	virtual void Yaw_Implementation(float value);
	virtual bool Yaw_Validate(float value);

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	float _turnSpeed = 20.0f;
	float _rollSpeed = 100.0f;
	float _moveSpeed = 10000.0f;
	float _strafeSpeed = 10000.0f;

	float pitchDelta = .0f;
	float yawDelta = .0f;
	float rollDelta = .0f;
	AMainCharacterController* _character = nullptr;

};
