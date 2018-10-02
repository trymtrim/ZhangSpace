// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

UCLASS()
class ZHANGSPACE_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY ()

public:
	virtual void SetupInputComponent () override;

	//Handles input for moving forward and backward
	UFUNCTION ()
	void MoveForward (float value);

	//Handles input for moving right and left
	UFUNCTION ()
	void MoveRight (float value);

	/// <param name="value">Changes the roll value in LocalRotation</param>  
	//Handles the roll feature of the spacecraft
	UFUNCTION ()
	void Roll (float value);

	//Handles the pitching feature of the spacecraft
	UFUNCTION ()
	void Pitch (float value);

	//Handles the yaw feature of the spacecraft
	UFUNCTION ()
	void Yaw (float value);
};
