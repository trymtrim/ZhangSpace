// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.h"
#include "Components/ArrowComponent.h"
#include "MainCharacterController.h"
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

	UFUNCTION (BlueprintImplementableEvent, Category = "AI")
	void DieBP ();

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

	virtual float TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	void ServerUpdate (float deltaTime);
	void UpdatePatrolState (float deltaTime);
	void UpdateAttackState (float deltaTime);

	bool CheckForAggro ();
	bool IsAttackableInScope ();

	void Shoot ();
	void Die ();

	enum State
	{
		PATROL,
		ATTACK
	};

	State _state = PATROL;
	AMainCharacterController* _target;

	int _health = 200;

	float _aggroRange = 12500.0f;
	float _loseAggroRange = 15000.0f;
	float _maxAttackCooldown = 0.5f;
	float _currentAttackCooldown = 0.0f;

	bool _gunPositionSwitch = true;

	float _shootOutOfRangeTimer = 0.0f;
	
	UArrowComponent* gunPositionOne;
	UArrowComponent* gunPositionTwo;
	
	UPROPERTY (EditAnywhere)
	TSubclassOf <AProjectile> _projectileBP;
};
