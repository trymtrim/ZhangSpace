// Copyright Team Monkey Business 2018.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShrinkingCircle.generated.h"

UCLASS()
class ZHANGSPACE_API AShrinkingCircle : public AActor
{
	GENERATED_BODY ()
	
public:	
	//Sets default values for this actor's properties
	AShrinkingCircle ();

	//Called every frame
	virtual void Tick (float DeltaTime) override;

	UPROPERTY (Replicated, BlueprintReadOnly)
	bool gameStarted = false;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay () override;

private:
	float _delayTimer = 0.0f;
	
	UPROPERTY (EditAnywhere)
	AActor* _objectToSurround;
	UPROPERTY (EditAnywhere)
	float _shrinkSpeed;
	UPROPERTY (EditAnywhere)
	float _delayTime;
	UPROPERTY (EditAnywhere)
	float _endRadius;
};
