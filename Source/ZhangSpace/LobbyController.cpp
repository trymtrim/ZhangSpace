// Copyright Team Monkey Business 2018.

#include "LobbyController.h"
#include "ConfigManager.h"

//Sets default values
ALobbyController::ALobbyController ()
{
 	//Set this actor to call Tick () every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void ALobbyController::BeginPlay ()
{
	Super::BeginPlay ();

	//Declare the IP address of the master server
	ipAddress = ConfigManager::GetConfig ("IP_Address");
	playerName = ConfigManager::GetConfig ("Player_Name");
}

//Called every frame
void ALobbyController::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);
}

void ALobbyController::SetPlayerName (FString newPlayerName)
{
	ConfigManager::ChangeConfig ("Player_Name", newPlayerName);
	playerName = newPlayerName;
}
