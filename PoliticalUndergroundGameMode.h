// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PoliticalUndergroundCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "BasePlayerController.h"
#include "PoliticalUndergroundGameMode.generated.h"




// Copyright Epic Games, Inc. All Rights Reserved.





UCLASS(minimalapi)
class APoliticalUndergroundGameMode : public AGameModeBase
{
	GENERATED_BODY()


public:
	APoliticalUndergroundGameMode();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
	ABasePlayerController* P1Controller; // Local
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
	ABasePlayerController* P2Controller; // Remote


	void PostLogin(APlayerController* NewPlayer) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void GenericPlayerInitialization(AController* Controller) override;


 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player References")
	APoliticalUndergroundCharacter* Player1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player References")
	APoliticalUndergroundCharacter* Player2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int RoundsToWin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int PlayerOneWins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	int PlayerTwoWins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode Settings")
	float roundTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode Settings")
	float defaultRoundTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode Settings")
	int numRounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode Settings")
	bool isTimerActive;

	int PlayerNum;

	UFUNCTION()
	void OnControllerChanged(EInputDeviceConnectionState connectionState, FPlatformUserId userID, FInputDeviceId inputDeviceID);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowServerList();

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnNetworkPawns();

	UFUNCTION()
	void AddBaseController(ABasePlayerController* controller);

	UFUNCTION()
	void CheckControllerRef(ABasePlayerController* controller);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Connected Controllers")
	TArray<ABasePlayerController*> Game_Controllers;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Connected Controllers")
	TArray<APlayerController*> Connected_Controllers;

};



