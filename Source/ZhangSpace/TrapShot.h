// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrapShot.generated.h"

UCLASS()
class ZHANGSPACE_API ATrapShot : public AActor
{
	GENERATED_BODY ()
	
public:	
	//Sets default values for this actor's properties
	ATrapShot ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	UFUNCTION (BlueprintCallable)
	void DestroyField ();

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

private:
	void ServerUpdate (float deltaTime);

	bool _destroyed = false;
};
