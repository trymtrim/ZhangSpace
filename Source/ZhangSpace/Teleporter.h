// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Teleporter.generated.h"

UCLASS()
class ZHANGSPACE_API ATeleporter : public AActor
{
	GENERATED_BODY ()
	
public:
	//Sets default values for this actor's properties
	ATeleporter ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	UFUNCTION (BlueprintCallable)
	void TeleportPlayer (AActor* player);

	UPROPERTY (BlueprintReadOnly) AActor* secondTeleporter;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

private:
	void ServerUpdate (float deltaTime);
	void SpawnSecondTeleporter ();

	float _destroyTimer = 0.0f;

	UPROPERTY (EditAnywhere)
	TSubclassOf <AActor> _secondTeleporterBP;
	UPROPERTY (EditAnywhere)
	float _duration = 5.0f;
	UPROPERTY (EditAnywhere)
	float _distance = 5000.0f;
};
