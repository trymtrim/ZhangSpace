// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Projectile.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "ShrinkingCircle.h"
#include "Shield.h"
#include "Components/ArrowComponent.h"
#include "Runtime/UMG/Public/Components/PanelWidget.h"
#include "MainCharacterController.generated.h"

class AMainGameState;

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
	UFUNCTION (BlueprintImplementableEvent, Category = "Ability Menu")
	void OpenAbilityMenuBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Ability Menu")
	void CloseAbilityMenuBP ();
	UFUNCTION (BlueprintCallable)
	void CloseAbilityMenu ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void DieBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void RespawnBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void TakeDamageBP (int damage, const FString& damageType);
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void SpawnShieldBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void ShieldTakeDamageBP ();
	UFUNCTION(BlueprintImplementableEvent, Category = "Character Controller")
	void ShootBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void CloakBP ();
	UFUNCTION (BlueprintCallable)
	void UpdateHotkeyBar (TArray <int> abilities);
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void UpdateHotkeyBarBP (const TArray <int>& hotkeyAbilities);
	UFUNCTION (BlueprintCallable)
	void ReplacePanelChild (UWidget* newWidget, UPanelWidget* panel, int index);
	UFUNCTION (BlueprintCallable)
	void SwitchPanelPosition (UWidget* widgetOne, UWidget* widgetTwo, UPanelWidget* panel);
	UFUNCTION (BlueprintCallable)
	void OpenSettingsMenu (bool open);

	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void StartSpectatingBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void ChangeSpectateTargetBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void SpectateRotateBP (const FString& rotateType, float value);

	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void AddFeedTextBP (const FString& rotateType);

	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void PlayUpgradeSoundBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void ServerPlayUpgradeSoundBP ();

	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void AfterburnerBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void ShockwaveBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void TrapShotBP (FVector hitPosition);
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void DetonateTrapShotBP ();
	
	UFUNCTION (Client, Reliable)
	void UpdateFeedText (const FString& feedText);

	UFUNCTION (Client, Reliable)
	void SetShieldHealth (int health);

	UPROPERTY (BlueprintReadOnly) bool isSpectating = false;

	UPROPERTY (Replicated, BlueprintReadOnly) int systemLevel = 1;

	UFUNCTION (BlueprintCallable)
	int GetPlayerID ();

	UPROPERTY (Replicated) int playerID = -1;

	bool GetCanMove ();

	UFUNCTION (BlueprintCallable)
	bool GetIsDead ();

	int GetMobilityPower ();

	UFUNCTION (BlueprintCallable)
	bool GetGameOver ();

	UFUNCTION (BlueprintCallable)
	bool IsAttackableInScope ();

	//Passive ability getters
	UFUNCTION (BlueprintCallable) 
	bool GetShieldReflect ();

	UPROPERTY (BlueprintReadOnly) bool shieldRam;

	void Disarm ();

	UFUNCTION (BlueprintCallable)
	void StartTrapShotCooldown ();

	UPROPERTY (Replicated, BlueprintReadOnly)
	bool disarmed = false;

	FVector2D GetViewportSize (); //Returns the size of the clients viewport as a 2d vector

	//Called from the server when the game starts
	void StartGame ();

	UPROPERTY (Replicated, BlueprintReadOnly) bool gameStarted = false;

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
	UPROPERTY (BlueprintReadOnly) int shieldHealth = 50;

	UPROPERTY (BlueprintReadOnly) bool cloakActive = false;

	//These variables are set by the shield script
	UPROPERTY (Replicated, BlueprintReadOnly) bool shieldActive = false;
	AShield* shield = nullptr;

	//Hit indicator
	UPROPERTY (Replicated, BlueprintReadOnly) FString playerHitText;
	int lastPlayerHitID = 0;
	int lastDamage = 0;

	float resetPlayerHitTimer = 0.0f;

	void UpdatePlayerHitText (int id, int damage);

	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void ResetPlayerHitTextBP ();

	UFUNCTION (BlueprintCallable)
	void UpdatePlayerHitTextFromBP (int id, int damage);

	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void StartHyperBeamBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void StopHyperBeamBP ();

	void DealBeamDamage (float damage, AMainCharacterController* player);

	bool GetChannelingBeam ();
	bool GetBoost ();

	UPROPERTY (Replicated, BlueprintReadOnly)
	FVector beamTargetPosition;

	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void HeatseekerBP (AMainCharacterController* playerTarget, int damage);

	UPROPERTY (Replicated)
	bool flyingIn = true;

	UFUNCTION (BlueprintCallable)
	void FinishFlyingIn ();

	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void FinishGameBP (const FString& winnerName);

	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void StartBeamSoundBP ();
	UFUNCTION (BlueprintImplementableEvent, Category = "Character Controller")
	void StopBeamSoundBP ();

	void FinishGame (FString winnerName);

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
	void Shoot (FVector cameraPosition, FVector cameraForward);
	void UseAbilityInput (int abilityIndex);

	template <int index>
	void UseAbilityInput ()
	{
		UseAbilityInput (index);
	}

	//Abilities
	void Shield ();
	void Cloak ();
	void Teleport ();
	void HyperBeam ();
	void Heatseeker ();
	void Shockwave ();
	void Afterburner ();
	void TrapShot (FVector cameraPosition);

	void DetonateTrapShot ();
	void CancelBoost ();

	//Passive abilities
	bool _shieldReflect = false;

	void DoTeleport ();

	void InitializeAbilityCooldowns ();
	void ChangeMesh (UStaticMesh* mesh);
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

	UFUNCTION (Client, Reliable)
	void ClientChangeShieldCooldown (int cooldown);

	UFUNCTION (Server, Reliable, WithValidation)
	void ChannelHyperBeam (FVector cameraPosition, FVector forwardVector);

	void CancelHyperBeam ();

	UPROPERTY (Replicated) bool _channelingBeam = false;

	bool _playingBeamSound = false;

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
	float _maxShootingCooldown = 0.3f;
	UPROPERTY (Replicated) float _currentShootingCooldown = 0.0f;

	int _level = 1;
	UPROPERTY (Replicated) int _lives = 3;
	int _maxStatPower = 10;
	float _shootCost = 2.5f;

	float _beamDamage = 0;
	float _beamDamageTimer = 0.0f;

	bool _isBoosting = false;
	bool _isUsingTrapShot = false;

	void CancelDisarm ();

	float _maxDeadTimer = 5.0f;
	UPROPERTY (Replicated) float _currentDeadTimer = 0.0f;

	UPROPERTY (Replicated) TArray <int> _abilities;
	TArray <int> _hotkeyBarAbilities;
	UPROPERTY (Replicated) TArray <float> _abilityCooldowns;
	TMap <int, float> _abilityMaxCooldowns;

	UPROPERTY (Replicated) bool _showCursor = false;
	bool _inAbilityMenu = false;
	bool _inSettingsMenu = false;

	int FPS = 0;
	void UpdateFPS ();

	UPROPERTY (Replicated) bool _gameFinished = false;

	UPROPERTY (Replicated) bool _gameOver = false;

	UCameraComponent* _cameraComponent;

	bool _gunPositionSwitch = true;

	UArrowComponent* gunPositionOne;
	UArrowComponent* gunPositionTwo;

	AMainGameState* _gameState = nullptr;

	UPROPERTY (EditAnywhere)
	UStaticMesh* _cockpitMesh;
	UPROPERTY (EditAnywhere)
	TSubclassOf <AProjectile> _projectileBP;
	UPROPERTY (EditAnywhere)
	TSubclassOf <AActor> _teleporterBP;
};
