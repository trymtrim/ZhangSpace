// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Projectile.h"
#include "MainCharacterController.generated.h"

UCLASS()
class ZHANGSPACE_API AMainCharacterController : public ACharacter
{
	GENERATED_BODY ()

public:
	AMainCharacterController ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	//Called to bind functionality to input
	virtual void SetupPlayerInputComponent (class UInputComponent* PlayerInputComponent) override;

	//Handles input for moving forward and backward
    UFUNCTION ()
    void MoveForward (float value);

    //Handles input for moving right and left
    UFUNCTION ()
    void MoveRight (float value);

	//Variables for the spaceship UI
	UPROPERTY (BlueprintReadOnly) float healthPercentage;
	UPROPERTY (BlueprintReadOnly) float powerPercentage;
	UPROPERTY (BlueprintReadOnly) float attackPowerPercentage;
	UPROPERTY (BlueprintReadOnly) float defensePowerPercentage;
	UPROPERTY (BlueprintReadOnly) float mobilityPowerPercentage;
	UPROPERTY (BlueprintReadOnly) float shieldCooldownPercentage;
	UPROPERTY (BlueprintReadOnly) FString healthText;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

	virtual float TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	UFUNCTION (Server, Reliable, WithValidation)
	void Shoot ();
	UFUNCTION (Server, Reliable, WithValidation)
	void Shield ();

	void UpdateStats (float deltaTime);

	void UpdateStatsUI ();

	//Player stats
	UPROPERTY (Replicated) int _maxHealth = 100;
	UPROPERTY (Replicated) int _currentHealth = 100;
	UPROPERTY (Replicated) float _maxPower = 100;
	UPROPERTY (Replicated) float _power = 100;
	UPROPERTY (Replicated) int _attackPower = 20;
	UPROPERTY (Replicated) int _defensePower = 20;
	UPROPERTY (Replicated) int _mobilityPower = 20;
	UPROPERTY (Replicated) float _maxShieldCooldown = 10.0f;
	UPROPERTY (Replicated) float _currentShieldCooldown = 0.0f;
	UPROPERTY (Replicated) bool _dead = false;

	int _maxStatPower = 100;

	float _shootCost = 2.5f;

	UPROPERTY (EditAnywhere)
	TSubclassOf <AProjectile> _projectileBP;
};
