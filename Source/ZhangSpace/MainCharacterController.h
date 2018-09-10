// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacterController.generated.h"

UCLASS()
class ZHANGSPACE_API AMainCharacterController : public ACharacter
{
	GENERATED_BODY ()

public:
	AMainCharacterController ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	//Called to bind functionality to input
	virtual void SetupPlayerInputComponent (class UInputComponent* PlayerInputComponent) override;

	// Handles input for moving forward and backward.
    UFUNCTION ()
    void MoveForward (float value);

    // Handles input for moving right and left.
    UFUNCTION ()
    void MoveRight (float value);

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;
};
