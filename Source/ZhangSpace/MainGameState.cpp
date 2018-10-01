// Copyright Team Monkey Business 2018.

#include "MainGameState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "MainPlayerController.h"
#include "GameFramework/Character.h"

AMainGameState::AMainGameState ()
{
	//Set this character to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void AMainGameState::BeginPlay ()
{
	Super::BeginPlay ();

	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
	{
		//Find available spells from scene
		TArray <AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass (GetWorld (), AShrinkingCircle::StaticClass (), actors);
		_shrinkingCircle = dynamic_cast <AShrinkingCircle*> (actors [0]);
	}
}

//Called every frame
void AMainGameState::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
		ServerUpdate (DeltaTime);
}

void AMainGameState::ServerUpdate (float deltaTime)
{
	//Update damage timer
	_damageTimer += deltaTime;

	//Damage everyone player who is outside of the circle based on damage interval
	if (_damageTimer >= _damageInterval)
	{
		DamagePlayersOutsideOfCircle ();
		_damageTimer = 0.0f;
	}
}

void AMainGameState::DamagePlayersOutsideOfCircle ()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld ()->GetPlayerControllerIterator (); Iterator; ++Iterator)
	{
		AMainPlayerController* playerController = Cast <AMainPlayerController> (*Iterator);

		if (playerController)
		{
			if (FVector::Distance (_shrinkingCircle->GetActorLocation (), playerController->GetCharacter ()->GetActorLocation ()) > _shrinkingCircle->GetActorScale3D ().X * 50.0f)
			{
				float damage = 2.5f;

				UGameplayStatics::ApplyDamage (playerController->GetCharacter (), damage, nullptr, nullptr, nullptr);
			}
		}
	}
}
