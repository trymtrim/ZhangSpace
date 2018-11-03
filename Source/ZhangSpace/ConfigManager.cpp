// Copyright Team Monkey Business 2018.

#include "ConfigManager.h"
#include "Engine/World.h"
#include <string>
#include <fstream>
#include "Runtime/Core/Public/Templates/SharedPointer.h"

FString ConfigManager::_configFilePath;

void ConfigManager::InitializeConfigFile (EWorldType::Type gameType)
{
	//Set config file path depending on whether we are in the editor or standalone
	if (gameType == EWorldType::Game)
		_configFilePath = FPaths::ConvertRelativePathToFull (FPaths::ConvertRelativePathToFull (FPaths::RootDir ())) + "config.json";
	else
		_configFilePath = FPaths::ConvertRelativePathToFull (FPaths::ConvertRelativePathToFull (FPaths::GetProjectFilePath ())) + "/../config.json";

	//Declare the file to check for
	std::ifstream fileToCheck (std::string (TCHAR_TO_UTF8 (*_configFilePath)));
	
	//If there is no file yet, create a file
	if (fileToCheck)
		return;

	FJsonObject* jsonObject = new FJsonObject ();
	jsonObject->SetStringField ("IP_Address", "127.0.0.1");
	jsonObject->SetStringField ("Player_Name", "Default Name");
	jsonObject->SetStringField ("Show_FPS", "false");
	jsonObject->SetStringField ("Show_Ping", "false");

	FString outputString;
	TSharedRef <TJsonWriter <>> writer = TJsonWriterFactory <>::Create (&outputString);
	FJsonSerializer::Serialize (MakeShareable (jsonObject), writer);

	//Save the file to disk
	std::ofstream file;
	file.open (std::string (TCHAR_TO_UTF8 (*_configFilePath)));
	file << std::string (TCHAR_TO_UTF8 (*outputString));
	file.close ();
}

void ConfigManager::ChangeConfig (FString configType, FString newConfig)
{
	std::string str;

	//Load the file
	std::ifstream loadFile (std::string (TCHAR_TO_UTF8 (*_configFilePath)));

	FString jsonString;

	//Read the next line from the file and add it to the json string until it reaches the end
	while (std::getline (loadFile, str))
	{
		jsonString += str.c_str ();
		jsonString += "\n";
	}

	TSharedPtr <FJsonObject> json;
	TSharedRef <TJsonReader <>> reader = TJsonReaderFactory<>::Create (jsonString);

	FJsonSerializer::Deserialize (reader, json);

	//Update the config attribute
	FJsonObject* jsonObject = new FJsonObject (*json);
	jsonObject->SetStringField (configType, newConfig);

	FString outputString;
	TSharedRef <TJsonWriter <>> writer = TJsonWriterFactory <>::Create (&outputString);
	FJsonSerializer::Serialize (MakeShareable (jsonObject), writer);
	
	//Save the file to disk
	std::ofstream file;
	file.open (std::string (TCHAR_TO_UTF8 (*_configFilePath)));
	file << std::string (TCHAR_TO_UTF8 (*outputString));
	file.close ();
}

FString ConfigManager::GetConfig (FString configType)
{
	std::string str;

	//Load the file
	std::ifstream loadFile (std::string (TCHAR_TO_UTF8 (*_configFilePath)));

	FString jsonString;

	//Read the next line from the file and add it to the json string until it reaches the end
	while (std::getline (loadFile, str))
	{
		jsonString += str.c_str ();
		jsonString += "\n";
	}

	TSharedPtr <FJsonObject> json;
	TSharedRef <TJsonReader <>> reader = TJsonReaderFactory<>::Create (jsonString);

	FJsonSerializer::Deserialize (reader, json);
	
	return json.Get ()->GetStringField (configType);
}
