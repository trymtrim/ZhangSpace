// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Projectile.h"
#include "Components/StaticMeshComponent.h"
#include "ShrinkingCircle.h"
#include "Shield.h"
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

	UFUNCTION (BlueprintCallable)
	void AddExperience (int experience);
	UFUNCTION (BlueprintCallable)
	void AddAbility (int abilityIndex);

	//Variables for the spaceship UI
	UPROPERTY (BlueprintReadOnly) float healthPercentage;
	UPROPERTY (BlueprintReadOnly) float powerPercentage;
	UPROPERTY (BlueprintReadOnly) float attackPowerPercentage;
	UPROPERTY (BlueprintReadOnly) float defensePowerPercentage;
	UPROPERTY (BlueprintReadOnly) float mobilityPowerPercentage;
	UPROPERTY (BlueprintReadOnly) float experiencePercentage;
	UPROPERTY (BlueprintReadOnly) FString healthText;
	UPROPERTY (BlueprintReadOnly) int availableStats;
	UPROPERTY (BlueprintReadOnly) bool attackUpgradeAvailable = false;
	UPROPERTY (BlueprintReadOnly) bool defenseUpgradeAvailable = false;
	UPROPERTY (BlueprintReadOnly) bool mobilityUpgradeAvailable = false;

	UPROPERTY (BlueprintReadOnly) float shieldCooldownPercentage;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

	virtual float TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	UFUNCTION (Server, Reliable, WithValidation)
	void Shoot ();
	UFUNCTION (Server, Reliable, WithValidation)
	void UseAbility (int abilityIndex);

	void UseAbilityInput (int abilityIndex);

	template <int index>
	void UseAbilityInput ()
	{
		UseAbilityInput (index);
	}

	//Abilities
	void Shield ();

	void ChangeMesh ();
	void AddAvailableStats ();
	void UpdateStats (float deltaTime);
	void UpdateStatsUI ();

	UFUNCTION (Server, Reliable, WithValidation)
	void AddStat (int statIndex);

	template <int index>
	void AddStat ()
	{
		AddStat (index);
	}

	//Player stats
	UPROPERTY (Replicated) int _maxHealth = 100;
	UPROPERTY (Replicated) int _currentHealth = 100;
	UPROPERTY (Replicated) int _attackPower = 1;
	UPROPERTY (Replicated) int _defensePower = 1;
	UPROPERTY (Replicated) int _mobilityPower = 1;
	UPROPERTY (Replicated) float _maxPower = 100;
	UPROPERTY (Replicated) float _power = 100;
	UPROPERTY (Replicated) int _experience = 0;
	UPROPERTY (Replicated) int _experienceToNextLevel = 100;
	UPROPERTY (Replicated) int _availableStats = 0;
	UPROPERTY (Replicated) bool _attackUpgradeAvailable = false;
	UPROPERTY (Replicated) bool _defenseUpgradeAvailable = false;
	UPROPERTY (Replicated) bool _mobilityUpgradeAvailable = false;
	UPROPERTY (Replicated) bool _dead = false;

	UPROPERTY (Replicated) float _maxShieldCooldown = 10.0f;
	UPROPERTY (Replicated) float _currentShieldCooldown = 0.0f;

	int _level = 1;
	int _maxStatPower = 10;
	float _shootCost = 2.5f;

	TArray <int> _abilities;

	UPROPERTY (EditAnywhere)
	UStaticMesh* _cockpitMesh;
	UPROPERTY (EditAnywhere)
	TSubclassOf <AProjectile> _projectileBP;
	UPROPERTY (EditAnywhere)
	TSubclassOf <AShield> _shieldBP;
};
