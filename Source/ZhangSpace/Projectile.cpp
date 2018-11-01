// Copyright Team Monkey Business 2018.

#include "Projectile.h"
#include "Engine/World.h"

//Sets default values
AProjectile::AProjectile ()
{
 	//Set this actor to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void AProjectile::BeginPlay ()
{
	Super::BeginPlay ();
}

//Called every frame
void AProjectile::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);
	
	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
		ServerUpdate (DeltaTime);
}

void AProjectile::ServerUpdate (float deltaTime)
{
	//Destroy projectile after a little while
	_destroyTimer += deltaTime;

	if (_destroyTimer >= 15.0f)
		Destroy ();
}

void AProjectile::SetDamage (float projectileDamage)
{
	damage = projectileDamage;
}
