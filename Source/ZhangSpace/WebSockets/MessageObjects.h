// Copyright Team Monkey Business 2018.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine.h"
#include "MessageObjects.generated.h"

UCLASS()
class ZHANGSPACE_API UMessageObjects : public UBlueprintFunctionLibrary
{
	GENERATED_BODY ()
};

UCLASS(BlueprintType, Blueprintable)
class UTypeCheck : public UObject
{
	GENERATED_BODY ()

public:
	UTypeCheck ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
};

//REQUEST OBJECTS

UCLASS(BlueprintType, Blueprintable)
class UCreateGameRequest : public UObject
{
	GENERATED_BODY ()

public:
	UCreateGameRequest ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString gameName;
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString playerName;
};

UCLASS(BlueprintType, Blueprintable)
class UJoinGameRequest : public UObject
{
	GENERATED_BODY ()

public:
	UJoinGameRequest ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString gameName;
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString playerName;
};

UCLASS(BlueprintType, Blueprintable)
class URefreshRequest : public UObject
{
	GENERATED_BODY ()

public:
	URefreshRequest ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
};

UCLASS(BlueprintType, Blueprintable)
class UFindGameRequest : public UObject
{
	GENERATED_BODY ()

public:
	UFindGameRequest ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
};

UCLASS(BlueprintType, Blueprintable)
class UCancelFindingGameRequest : public UObject
{
	GENERATED_BODY ()

public:
	UCancelFindingGameRequest ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
};

UCLASS(BlueprintType, Blueprintable)
class ULeaveLobbyRequest : public UObject
{
	GENERATED_BODY ()

public:
	ULeaveLobbyRequest ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
};

UCLASS(BlueprintType, Blueprintable)
class UStartGameRequest : public UObject
{
	GENERATED_BODY ()

public:
	UStartGameRequest ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
};

//RESPONSE OBJECTS

UCLASS(BlueprintType, Blueprintable)
class UJoinGameResponse : public UObject
{
	GENERATED_BODY ()

public:
	UJoinGameResponse ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString ipAddress;
};

UCLASS(BlueprintType, Blueprintable)
class URefreshResponse : public UObject
{
	GENERATED_BODY ()

public:
	URefreshResponse ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	int gameServerCount;
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	TArray <FString> gameServerIpAddresses;
};

UCLASS(BlueprintType, Blueprintable)
class URefreshLobbyResponse : public UObject
{
	GENERATED_BODY ()

public:
	URefreshLobbyResponse ();
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString type;
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	FString gameName;
	UPROPERTY (EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = MessageObjects)
	TArray <FString> lobbyNames;
};
