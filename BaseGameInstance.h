// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../PoliticalUndergroundCharacter.h"
#include "PoliticalUndergroundGameMode.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online.h"

#include "BaseGameInstance.generated.h"

/**
 * 
 */



UENUM(BlueprintType)
enum class EGameMode: uint8{
	VE_Story UMETA(DisplayName = "Story"),
	VE_SinglePlayer UMETA(DisplayName = "SinglePlayer"),
	VE_MultiPlayer UMETA(DisplayName = "MultiPlayer"),
	VE_Training UMETA(DisplayName = "Training"),
	VE_HostSession UMETA(DisplayName = "HostSession"),
	VE_JoinSession UMETA(DisplayName = "JoinSession"),

};

UENUM(BlueprintType)
enum class ECHARACTERCLASS : uint8{
	VE_None UMETA(DisplayName = "None"),
	VE_SleepyJoe UMETA(DisplayName = "SleepyJoe"),
	VE_SleepyJoeAI UMETA(DisplayName = "SleepyJoeAI"),
	VE_Donald UMETA(DisplayName = "Donald"),
	VE_DonaldAI UMETA(DisplayName = "DonaldAI"),

};


UENUM(BlueprintType)
enum class EMAPCLASS : uint8{
	VE_None UMETA(DisplayName = "None"),
	VE_WhiteHouse UMETA(DisplayName = "WhiteHouse"),
	VE_UnderGroundFightClub UMETA(DisplayName = "UnderGroundFightClub"),
};

UCLASS()
class POLITICALUNDERGROUND_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	
	void BeginPlay();

	UBaseGameInstance();

	void ShowGameServerList();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
	bool isDeviceForMultiplePlayers;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player References")
	ECHARACTERCLASS characterClass_PlayerOne;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player References")
	ECHARACTERCLASS characterClass_PlayerTwo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map References")
	EMAPCLASS Map_Selected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Streaming")
	FName LevelToLoad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	bool ShowDiagnostics = false;

	// Switch to GameMode?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	bool GameStarted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	bool ReturnToCharacterSelect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	bool MatchOver;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	EGameMode currentModeSelection;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> SessionNames;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> PingList;

	

	bool HostOnlineSession(FUniqueNetIdRepl UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);

		/* Delegate called when session created */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	/* Delegate called when session started */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

	/** Handles to registered delegates for creating/starting a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	TSharedPtr<class FOnlineSessionSettings> SessionSettings;

	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

	void FindOnlineSessions(FUniqueNetIdRepl UserId, bool bIsLAN, bool bIsPresence);

	/** Delegate for searching for sessions */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;

	/** Handle to registered delegate for searching a session */
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	FOnlineSessionSearchResult ServerToJoin;

	void OnFindSessionsComplete(bool bWasSuccessful);

	bool JoinOnlineSession(FUniqueNetIdRepl UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

	/** Delegate for joining a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

/** Handle to registered delegate for joining a session */
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);


	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

/** Handle to registered delegate for destroying a session */
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;

	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);


	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void StartOnlineGame();


	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void FindOnlineGames();

	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void JoinOnlineGame(int server_int);

	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void DestroySessionAndLeaveGame();

};