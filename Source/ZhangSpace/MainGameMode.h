// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainPlayerController.h"
#include "MainGameMode.generated.h"

UCLASS()
class ZHANGSPACE_API AMainGameMode : public AGameMode
{
	GENERATED_BODY ()
	
public:
	AMainGameMode ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	virtual AActor* ChoosePlayerStart_Implementation (AController* Player) override;
	void RegisterPlayer (AMainPlayerController* playerController, int targetPlayerCount);

private:
	void StartGame ();

	int _maxPlayers = 6;
	int _playerCount = 0;
	int _currentPlayerStartIndex = 0;

	TArray <AMainPlayerController*> _connectedPlayers;
	bool _gameStarted = false;

	int _targetPlayerCount = 0;

	float _connectedPlayersCheckTimer = 0.0f;

	float _canStopServerTimer = 0.0f;
	bool _canStopServer = false;
};
