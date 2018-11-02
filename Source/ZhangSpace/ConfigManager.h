// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"

class EWorld;

class ZHANGSPACE_API ConfigManager
{
public:
	//Make sure to run this method at the very start of the game
	static void InitializeConfigFile (EWorldType::Type gameType);

	static void ChangeConfig (FString configType, FString newConfig);
	static FString GetConfig (FString configType);

private:
	static FString _configFilePath;
};
