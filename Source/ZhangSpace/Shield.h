// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.h"
#include "Shield.generated.h"

class AMainCharacterController;

UCLASS()
class ZHANGSPACE_API AShield : public AActor
{
	GENERATED_BODY ()
	
public:	
	//Sets default values for this actor's properties
	AShield ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	void OnHitByProjectile (AActor* targetToFollow, int damage);

	UPROPERTY (BlueprintReadOnly) bool shieldReflect;

	void ApplyDamage (int damage);

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

	virtual float TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	void ServerUpdate (float deltaTime);

	float _destroyTimer = 0.0f;
	float _shieldDuration = 30.0f;

	int _health = 50;

	AMainCharacterController* parentPlayer;

	UPROPERTY (EditAnywhere)
	TSubclassOf <AActor> _projectileBP;
};
