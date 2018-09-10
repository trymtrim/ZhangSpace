// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacterController.h"

AMainCharacterController::AMainCharacterController ()
{
 	//Set this character to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void AMainCharacterController::BeginPlay ()
{
	Super::BeginPlay ();
}

//Called every frame
void AMainCharacterController::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);
}

void AMainCharacterController::MoveForward (float value)
{
    //Find out which way is forward and record that the player wants to move that way
    FVector direction = FRotationMatrix (Controller->GetControlRotation ()).GetScaledAxis (EAxis::X);
    AddMovementInput (direction, value);
}

void AMainCharacterController::MoveRight (float value)
{
    //Find out which way is right and record that the player wants to move that way
    FVector direction = FRotationMatrix (Controller->GetControlRotation ()).GetScaledAxis (EAxis::Y);
    AddMovementInput (direction, value);
}

//Called to bind functionality to input
void AMainCharacterController::SetupPlayerInputComponent (UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent (PlayerInputComponent);

	//Set up movement bindings
    PlayerInputComponent->BindAxis ("MoveForward", this, &AMainCharacterController::MoveForward);
    PlayerInputComponent->BindAxis ("MoveRight", this, &AMainCharacterController::MoveRight);
}
