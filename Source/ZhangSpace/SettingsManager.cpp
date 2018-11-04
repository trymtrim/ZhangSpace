// Copyright Team Monkey Business 2018.

#include "SettingsManager.h"
#include "ConfigManager.h"
#include "Engine/World.h"

void USettingsManager::Init ()
{
	//Initialize the game's config file
	ConfigManager::InitializeConfigFile (GetWorld ()->WorldType);

	if (ConfigManager::GetConfig ("Show_FPS") == "true")
		showFPS = true;
	else
		showFPS = false;

	if (ConfigManager::GetConfig ("Show_Ping") == "true")
		showPing = true;
	else
		showPing = false;
}

void USettingsManager::SetShowFPS (bool status)
{
	if (status)
		ConfigManager::ChangeConfig ("Show_FPS", "true");
	else
		ConfigManager::ChangeConfig ("Show_FPS", "false");

	showFPS = status;
}

void USettingsManager::SetShowPing (bool status)
{
	if (status)
		ConfigManager::ChangeConfig ("Show_Ping", "true");
	else
		ConfigManager::ChangeConfig ("Show_Ping", "false");

	showPing = status;
}
