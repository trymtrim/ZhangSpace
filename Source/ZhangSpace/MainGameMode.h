// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
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

private:
	int _maxPlayers = 6;
	int _playerCount = 0;
	int _currentPlayerStartIndex = 0;

	float _connectedPlayersCheckTimer = 0.0f;
};
