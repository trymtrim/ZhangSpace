// Copyright Team Monkey Business 2018.

#include "TrapShot.h"
#include "Engine/World.h"
#include "MainPlayerController.h"

//Sets default values
ATrapShot::ATrapShot ()
{
 	//Set this actor to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void ATrapShot::BeginPlay ()
{
	Super::BeginPlay ();
}

//Called every frame
void ATrapShot::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	if (GetWorld ()->IsServer ())
		ServerUpdate (DeltaTime);
}

void ATrapShot::ServerUpdate (float deltaTime)
{
	if (_destroyed)
		return;

	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
		{
			if (FVector::Distance (GetActorLocation (), playerController->GetCharacter ()->GetActorLocation ()) < 400.0f * 50.0f)
				playerController->_slowed = true;
			else
				playerController->_slowed = false;
		}
	}
}

void ATrapShot::DestroyField ()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
			playerController->_slowed = false;
	}

	_destroyed = true;
}
