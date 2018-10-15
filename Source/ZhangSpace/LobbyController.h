// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyController.generated.h"

UCLASS()
class ZHANGSPACE_API ALobbyController : public AActor
{
	GENERATED_BODY ()
	
public:	
	//Sets default values for this actor's properties
	ALobbyController ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	UPROPERTY (BlueprintReadOnly)
	FString ipAddress;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;
};
