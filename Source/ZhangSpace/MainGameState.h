// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShrinkingCircle.h"
#include "MainGameState.generated.h"

UCLASS()
class ZHANGSPACE_API AMainGameState : public AGameState
{
	GENERATED_BODY ()

public:
	AMainGameState ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

private:
	void ServerUpdate (float deltaTime);
	void DamagePlayersOutsideOfCircle ();

	float _damageInterval = 1.0f;
	float _damageTimer = 0.0f;

	UPROPERTY ()
	AShrinkingCircle* _shrinkingCircle;
};
