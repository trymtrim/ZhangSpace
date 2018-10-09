// Copyright Team Monkey Business 2018.

#include "Shield.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "MainCharacterController.h"

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

	if (!GetWorld ()->IsServer ())
	{
		AMainCharacterController* parentPlayer = Cast <AMainCharacterController> (GetAttachParentActor ());
		AMainCharacterController* localPlayer = Cast <AMainCharacterController> (GetWorld ()->GetFirstPlayerController ()->GetCharacter ());

		//If the parent player is the local player, turn off the mesh
		if (parentPlayer == localPlayer)
		{
			TArray <UStaticMeshComponent*> meshComps;
			GetComponents <UStaticMeshComponent> (meshComps);
			UStaticMeshComponent* meshComponent = meshComps [0];

			meshComponent->SetStaticMesh (nullptr);
		}
	}
	else
	{
		//Check if player has shield refleft, only on server side atm, variable is not replicated
		AMainCharacterController* parentPlayer = Cast <AMainCharacterController> (GetParentActor ());

		if (parentPlayer->GetShieldReflect ())
		{
			shieldReflect = true;
			GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, "Has shield reflect");
		}
	}
}

//Called every frame
void AShield::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
		ServerUpdate (DeltaTime);
}

void AShield::OnHitByProjectile (FRotator bulletRotation)
{
	if (shieldReflect)
	{/*
		//Declare spawn parameters
		FActorSpawnParameters spawnParams;
		FVector spawnPosition = GetActorLocation () - bulletRotation.Vector () * 50.0f; //GetInverse ().Vector ().ForwardVector * 50.0f;
		FRotator spawnRotation = (-bulletRotation.Vector ().ForwardVector).Rotation ();

		//Spawn projectile
		GetWorld ()->SpawnActor <AActor> (_projectileBP, spawnPosition, spawnRotation, spawnParams);*/
	}

	//GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, "Shield hit by projectile");
}

float AShield::TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	_health -= Damage;


	//If health is below zero, die
	if (_health <= 0)
		Destroy ();

	//Debug
	GEngine->AddOnScreenDebugMessage (-1, 15.0f, FColor::Yellow, "Health: " + FString::FromInt (_health));

	return Super::TakeDamage (Damage, DamageEvent, EventInstigator, DamageCauser);
}

void AShield::ServerUpdate (float deltaTime)
{
	//Destroy itself after a while
	_destroyTimer += deltaTime;

	if (_destroyTimer >= _shieldDuration)
		Destroy ();
}
