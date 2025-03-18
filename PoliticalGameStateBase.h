// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../BaseClasses/GlobalAtomicInt.h"
#include "UObject/StrongObjectPtr.h"
#include "GameFramework/GameStateBase.h"
#include "networking.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Net/UnrealNetwork.h"
#include "IPAddress.h"
#include "GGPOGameInstance.h"
#include "include/ggponet.h"
#include "../PoliticalUndergroundCharacter.h"
#include "PoliticalGameStateBase.generated.h"

/**
 * 
 */
// Runnable class for receiving UDP packets asynchronously





class UGGPONetwork;

#define NETWORK_GRAPH_STEPS 720
#define MAX_PLAYERS 2


USTRUCT(BlueprintType)
struct FightGameState {

    GENERATED_USTRUCT_BODY()

	CharacterStateInfo PlayerOne;
	CharacterStateInfo PlayerTwo;

	 FightGameState& operator=(const FightGameState& other) {
        if (this != &other) {
            // Copy each member from 'other' to 'this'
            // Example: this->member = other.member;
        }
        return *this;
    }
    
};



UENUM(BlueprintType)
enum class EMenuType : uint8
{
	VE_None UMETA(DisplayName = "None"),
	VE_Up UMETA(DisplayName = "Up"),
	VE_Down UMETA(DisplayName = "Down"),
	VE_Right UMETA(DisplayName = "Right"),
	VE_Left UMETA(DisplayName = "Left"),
	VE_Confirm UMETA(DisplayName = "Confirm"),
	VE_Back UMETA(DisplayName = "Back"),
	
	// must go back through .cpp file when sobererrr.... Lol

};




UCLASS()
class POLITICALUNDERGROUND_API APoliticalGameStateBase : public AGameStateBase
{
	GENERATED_BODY()



private:
	bool bSessionStarted;

	float ElapsedTime;

	FString Remote_Port;
	FString Remote_IP;

	const char* Remote_IP_Char;
	unsigned short Remote_Port_Short;


public:
	APoliticalGameStateBase();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;



	bool GGPO_Started;

	GGPOSession* ggpo = nullptr;

	FightGameState GS;

	int32 gTick;

	GGPOPlayer p1, p2;
    GGPOPlayerHandle player_handles[2];
	GGPOPlayerHandle Local_Player_Handle;


	int32 local_input;
	bool Controller_Set;



	void GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const;


	UFUNCTION(Server, Reliable, BlueprintCallable)
	void StartNetworking();
	void StartNetworking_Implementation();




	UFUNCTION(Client, Reliable)
	void GetConnectedPlayerNetworkInfo(int Port, const FString& IP);
	void GetConnectedPlayerNetworkInfo_Implementation(int Port, const FString& IP);


	UFUNCTION(Client, Reliable)
	void RetrieveLocalInputs(int32 input);
	void RetrieveLocalInputs_Implementation(int32 input);


	UFUNCTION(Client, Reliable)
	void SetClientCharacter(APoliticalUndergroundCharacter* Character);
	void SetClientCharacter_Implementation(APoliticalUndergroundCharacter* Character);

	UFUNCTION(Client, Reliable)
	void SetConnectedCharacter(APoliticalUndergroundCharacter* Character);
	void SetConnectedCharacter_Implementation(APoliticalUndergroundCharacter* Character);


	TStrongObjectPtr<APoliticalUndergroundCharacter> Client_Character;
	TStrongObjectPtr<APoliticalUndergroundCharacter> Connected_Character;

	bool Client_Character_Simulation_Set;
	bool Connected_Character_Simulation_Set;


	void GetCharacterStates();

	void GetCharacterRefs();

	void UpdateGameState(int32 inputs[], bool rollback);

	void TryStartGGPOSession();

	void PU_Idle(int32 time);

	void PU_RunFrame();

	void PU_AdvanceFrame(int32 inputs[], int32 disconnect_flags, bool IsRollback);

	void PU_GetNetworkStats(int type);


	uint32_t fletcher32_checksum(const short* data, size_t len);

	GGPOSessionCallbacks CreateCallBacks();

	bool pu_begin_game_callback(const char*);
	bool pu_save_game_state_callback(unsigned char** buffer, int32* len, int32* checksum, int32);
	bool pu_load_game_state_callback(unsigned char* buffer, int32 len);
	bool pu_log_game_state(char* filename, unsigned char* buffer, int32);
	void pu_free_buffer(void* buffer);
	bool pu_advance_frame_callback(int32);
	bool pu_on_event_callback(GGPOEvent* info);


	

};
