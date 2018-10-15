// Copyright Team Monkey Business 2018.

#include "LobbyController.h"
#include "Engine/World.h"
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

	//Initialize the game's config file
	ConfigManager::InitializeConfigFile (GetWorld ()->WorldType);

	//Declare the IP address of the master server
	ipAddress = ConfigManager::GetConfig ("IP_Address");
}

//Called every frame
void ALobbyController::Tick (float DeltaTime)
{
	Super::Tick (DeltaTime);
}
