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
	AMainCharacterController ();	//Cuntstroker...

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	//Called to bind functionality to input
	virtual void SetupPlayerInputComponent (class UInputComponent* PlayerInputComponent) override;

	UFUNCTION (BlueprintCallable)
	void AddExperience (int experience);
	UFUNCTION (BlueprintCallable)
	void AddAbility (int abilityIndex);
	UFUNCTION (BlueprintImplementableEvent, Category = "Ability Menu")
	void OpenAbilityMenuBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Ability Menu")
	void CloseAbilityMenuBP ();
	UFUNCTION (BlueprintCallable)
	void CloseAbilityMenu ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void DieBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void TakeDamageBP (int damage, const FString& damageType);
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void SpawnShieldBP ();
	UFUNCTION(BlueprintImplementableEvent, Category = "Character Controller")
	void ShootBP ();
	UFUNCTION (BlueprintCallable)
	void UpdateHotkeyBar (TArray <int> abilities);

	bool GetCanMove ();
	bool GetIsDead ();

	//Pasive ability getters
	bool GetShieldReflect ();

	FVector2D GetViewportSize (); //Returns the size of the clients viewport as a 2d vector

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
	UPROPERTY (BlueprintReadOnly) FString respawnText;
	UPROPERTY (BlueprintReadOnly) int lives;
	UPROPERTY (BlueprintReadOnly) TArray <float> cooldownPercentages {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	UPROPERTY (BlueprintReadOnly) FString FPSText;

	//These variables are set by the shield script
	UPROPERTY (Replicated, BlueprintReadOnly) bool shieldActive = false;
	AShield* shield = nullptr;

	//Temp for midterm
	UPROPERTY (BlueprintReadOnly) bool teleportUnlocked = false;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

	virtual float TakeDamage (float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	UFUNCTION (Server, Reliable, WithValidation)
	void StartShooting ();
	UFUNCTION (Server, Reliable, WithValidation)
	void StopShooting ();
	UFUNCTION (Server, Reliable, WithValidation)
	void UseAbility (int abilityIndex, FVector cameraPosition);
	UFUNCTION (Server, Reliable, WithValidation)
	void ServerAddAbility (int abilityIndex);

	void UpdateShooting ();
	void UpdateShootingCooldown (float deltaTime);
	void StartShootingInput ();
	UFUNCTION (Server, Reliable, WithValidation)
	void Shoot (FVector cameraPosition);
	void UseAbilityInput (int abilityIndex);

	template <int index>
	void UseAbilityInput ()
	{
		UseAbilityInput (index);
	}

	//Abilities
	void Shield ();
	void Teleport ();

	//Passive abilities
	bool _shieldReflect = false;

	void InitializeAbilityCooldowns ();
	void ChangeMesh ();
	void AddAvailableStats ();
	void UpdateStats (float deltaTime);
	void UpdateStatsUI ();
	void EnableMouseCursor ();
	void DisableMouseCursor ();
	void MouseClick ();
	void ToggleAbilityMenu ();

	UFUNCTION (Server, Reliable, WithValidation)
	void SetShowCursor (bool show);

	void Die ();
	void UpdateDeadState (float deltaTime);
	void Respawn ();
	void GameOver ();

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
	UPROPERTY (Replicated) float _maxPower = 50;
	UPROPERTY (Replicated) float _power = 50;
	UPROPERTY (Replicated) int _experience = 0;
	UPROPERTY (Replicated) int _experienceToNextLevel = 100;
	UPROPERTY (Replicated) int _availableStats = 0;
	UPROPERTY (Replicated) int _attackUpgradesAvailable = 0;
	UPROPERTY (Replicated) int _defenseUpgradesAvailable = 0;
	UPROPERTY (Replicated) int _mobilityUpgradesAvailable = 0;
	UPROPERTY (Replicated) bool _dead = false;

	UPROPERTY (Replicated) bool _shooting = false;
	float _maxShootingCooldown = 0.25f;
	UPROPERTY (Replicated) float _currentShootingCooldown = 0.0f;

	int _level = 1;
	UPROPERTY (Replicated) int _lives = 3;
	int _maxStatPower = 10;
	float _shootCost = 2.5f;

	float _maxDeadTimer = 5.0f;
	UPROPERTY (Replicated) float _currentDeadTimer = 0.0f;

	UPROPERTY (Replicated) TArray <int> _abilities;
	TArray <int> _hotkeyBarAbilities;
	UPROPERTY (Replicated) TArray <float> _abilityCooldowns;
	TMap <int, float> _abilityMaxCooldowns;

	UPROPERTY (Replicated) bool _showCursor = false;
	bool _inAbilityMenu = false;

	int FPS = 0;
	void UpdateFPS ();

	UPROPERTY (EditAnywhere)
	UStaticMesh* _cockpitMesh;
	UPROPERTY (EditAnywhere)
	TSubclassOf <AProjectile> _projectileBP;
	UPROPERTY (EditAnywhere)
	TSubclassOf <AActor> _teleporterBP;
};
