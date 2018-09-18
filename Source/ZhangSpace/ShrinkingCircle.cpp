// Copyright Team Monkey Business 2018.

#include "ShrinkingCircle.h"
#include "Engine/World.h"

AShrinkingCircle::AShrinkingCircle ()
{
 	//Set this actor to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void AShrinkingCircle::BeginPlay ()
{
	Super::BeginPlay ();
}

//Called every frame
void AShrinkingCircle::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);

	if (GetWorld ()->IsServer () && Role == ROLE_Authority)
	{
		//Shrink circle after the delay time is over (_shrinkSpeed = m/s)
		if (_delayTimer < _delayTime)
			_delayTimer += DeltaTime;
		else if (GetActorScale3D ().X > _endRadius)
			SetActorScale3D (GetActorScale3D () - FVector (_shrinkSpeed) * DeltaTime);

		//If circle is not at the position of the object it should surround, move it there
		if (GetActorLocation () != _objectToSurround->GetActorLocation ())
			SetActorLocation (_objectToSurround->GetActorLocation ());
	}
}
