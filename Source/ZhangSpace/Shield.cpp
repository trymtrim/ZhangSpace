// Copyright Team Monkey Business 2018.

#include "Shield.h"
#include "Engine/World.h"

//Sets default values
AShield::AShield ()
{
 	//Set this actor to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void AShield::BeginPlay ()
{
	Super::BeginPlay ();
}

//Called every frame
void AShield::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
		ServerUpdate (DeltaTime);
}

void AShield::SetShieldOwner (AActor* owner)
{
	_owner = owner;
}

void AShield::ServerUpdate (float deltaTime)
{
	//Follow the player
	if (_owner != nullptr)
		SetActorLocation (_owner->GetActorLocation ());

	//Destroy itself after a while
	_destroyTimer += deltaTime;

	if (_destroyTimer >= _shieldDuration)
		Destroy ();
}
