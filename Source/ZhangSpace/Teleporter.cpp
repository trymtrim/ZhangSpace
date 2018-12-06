// Copyright Team Monkey Business 2018.

#include "Teleporter.h"
#include "Engine/World.h"

//Sets default values
ATeleporter::ATeleporter ()
{
 	//Set this actor to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void ATeleporter::BeginPlay ()
{
	Super::BeginPlay ();
}

//Called every frame
void ATeleporter::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
		ServerUpdate (DeltaTime);
}

void ATeleporter::ServerUpdate (float deltaTime)
{
	//Destroy itself after a while
	_destroyTimer += deltaTime;

	if (_destroyTimer >= _duration)
		Destroy ();
}

void ATeleporter::SpawnSecondTeleporter ()
{
	//Declare spawn parameters
	FActorSpawnParameters spawnParams;
	FVector spawnPosition = GetActorLocation () + GetActorForwardVector () * _distance;
	FRotator spawnRotation = GetActorRotation ();

	//Spawn teleporter to land in
	//secondTeleporter = GetWorld ()->SpawnActor <AActor> (_secondTeleporterBP, spawnPosition, spawnRotation, spawnParams);
}

void ATeleporter::TeleportPlayer (AActor* player)
{
	player->SetActorLocation (player->GetActorLocation () + player->GetActorForwardVector () * 30000 + player->GetActorForwardVector () * 3000.0f);
}
