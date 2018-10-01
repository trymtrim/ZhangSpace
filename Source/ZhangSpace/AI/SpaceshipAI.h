// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.h"
#include "GameFramework/Character.h"
#include "SpaceshipAI.generated.h"

UCLASS()
class ZHANGSPACE_API ASpaceshipAI : public AActor
{
	GENERATED_BODY ()
	
public:	
	//Sets default values for this actor's properties
	ASpaceshipAI ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

private:
	void ServerUpdate (float deltaTime);
	void UpdatePatrolState (float deltaTime);
	void UpdateAttackState (float deltaTime);

	bool CheckForAggro ();

	void Shoot ();

	enum State
	{
		PATROL,
		ATTACK,
		DEAD
	};

	State _state = PATROL;
	UPROPERTY (EditAnywhere)
	ACharacter* _target;

	float _aggroRange = 1500.0f;
	float _loseAggroRange = 3000.0f;
	float _maxAttackCooldown = 0.5f;
	float _currentAttackCooldown = 0.0f;

	UPROPERTY (EditAnywhere)
	TSubclassOf <AProjectile> _projectileBP;
};
