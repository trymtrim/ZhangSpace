// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Shield.generated.h"

UCLASS()
class ZHANGSPACE_API AShield : public AActor
{
	GENERATED_BODY ()
	
public:	
	//Sets default values for this actor's properties
	AShield ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	void SetShieldOwner (AActor* owner);

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

private:
	void ServerUpdate (float deltaTime);

	float _destroyTimer = 0.0f;

	float _shieldDuration = 5.0f;

	AActor* _owner;
};
