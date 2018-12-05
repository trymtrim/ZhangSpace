// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class ZHANGSPACE_API AProjectile : public AActor
{
	GENERATED_BODY ()
	
public:	
	//Sets default values for this actor's properties
	AProjectile ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	UFUNCTION (blueprintCallable)
	void SetDamage (float projectileDamage);

	void SetFollowTarget (AActor* target);

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

	UPROPERTY (BlueprintReadOnly)
	float damage = 0.0f;
	UPROPERTY (BlueprintReadOnly)
	AActor* followTarget = nullptr;

private:
	void ServerUpdate (float deltaTime);

	float _destroyTimer = 0.0f;
};
