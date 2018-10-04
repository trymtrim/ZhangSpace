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

	virtual AActor* ChoosePlayerStart_Implementation (AController* Player) override;

private:
	int _maxPlayers = 6;
	int _playerCount = 0;
	int _currentPlayerStartIndex = 0;
};
