// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.h"
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

	UFUNCTION (BlueprintCallable)
	void OnHitByProjectile (FRotator bulletRotation);

	UPROPERTY (BlueprintReadOnly) bool shieldReflect;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

	virtual float TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	void ServerUpdate (float deltaTime);

	float _destroyTimer = 0.0f;
	float _shieldDuration = 30.0f;

	int _health = 100;

	AActor* _owner;

	UPROPERTY (EditAnywhere)
	TSubclassOf <AProjectile> _projectileBP;
};
