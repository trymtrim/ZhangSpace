// Fill out your copyright notice in the Description page of Project Settings.

#include "MainGameMode.h"
#include "MainPlayerController.h"
#include "MainGameState.h"

AMainGameMode::AMainGameMode ()
{
	//Set default pawn class
	static ConstructorHelpers::FClassFinder <APawn> PlayerPawnClass (TEXT ("/Game/MainCharacterController"));

	if (PlayerPawnClass.Class != NULL)
		DefaultPawnClass = PlayerPawnClass.Class;

	//Set default player controller class
	PlayerControllerClass = AMainPlayerController::StaticClass ();

	//Set default game state class
	GameStateClass = AMainGameState::StaticClass ();
}
