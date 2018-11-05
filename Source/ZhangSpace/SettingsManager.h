// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SettingsManager.generated.h"

UCLASS()
class ZHANGSPACE_API USettingsManager : public UGameInstance
{
	GENERATED_BODY ()

public:
	UFUNCTION (BlueprintCallable)
	void SetShowFPS (bool status);
	UFUNCTION (BlueprintCallable)
	void SetShowPing (bool status);

	UPROPERTY (BlueprintReadOnly)
	bool showFPS;
	UPROPERTY (BlueprintReadOnly)
	bool showPing;

	UFUNCTION (BlueprintCallable)
	void SetTargetPlayerCount (int targetPlayerCount);
	int GetTargetPlayerCount ();

protected:
	//Called when the game starts or when spawned
	virtual void Init () override;

private:
	int _targetPlayerCount = 0;
};
