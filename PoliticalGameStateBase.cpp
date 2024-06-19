// Fill out your copyright notice in the Description page of Project Settings.
#include "PoliticalGameStateBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EngineUtils.h" // Include the header for TActorIterator
#include "PoliticalUndergroundGameMode.h"



#define FRAME_RATE 60
#define ONE_FRAME (1.0f / FRAME_RATE)




APoliticalGameStateBase::APoliticalGameStateBase(){
    

    PrimaryActorTick.bCanEverTick = true;
    GGPO_Started = false;
    Controller_Set = false; 
  
    Connected_Character_Simulation_Set = false;
    Client_Character_Simulation_Set = false;
    gTick = 0;

}


void APoliticalGameStateBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ElapsedTime += DeltaTime;
    int32 IdleMs = (int32)(ONE_FRAME - (int32)(ElapsedTime * 1000));

    

    if(GGPO_Started){
        PU_GetNetworkStats(0);
        PU_Idle(FMath::Max(0, IdleMs - 1)); // must be called or else add input will return not synchronized error
        PU_RunFrame();
        gTick++;
        
    }


}




void APoliticalGameStateBase::PU_Idle(int32 time){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Calling GGPO Idle")));

    GGPONet::ggpo_idle(ggpo, time);
}

void APoliticalGameStateBase::BeginPlay(){
    Super::BeginPlay();
}


void APoliticalGameStateBase::StartNetworking_Implementation(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Start Networking")));
    GetConnectedPlayerNetworkInfo_Implementation();
}




void APoliticalGameStateBase::GetConnectedPlayerNetworkInfo_Implementation(){

   
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Get Network Info")));
    int size = PlayerArray.Num();
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Player Array Size: %d"), size));
    int i = 0;



    for(APlayerState* player : PlayerArray){
        UNetConnection* NetConnection = player->GetNetConnection();
        if(NetConnection){

            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("GS url is %s"), *NetConnection->URL.Host));
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("GS port is %d"), NetConnection->URL.Port));
            int Remote_Port_Int = NetConnection->URL.Port;
            Remote_IP = *NetConnection->URL.Host;
           
           // const FString IPAddress = NetConnection->RemoteAddressToString();
            //IPAddress.Split(TEXT(":"), &Remote_IP, &Remote_Port);
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("IP Address: %s"), *IPAddress));
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("IP: %s"), *Remote_IP));
           // GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Port: %s"), *Remote_Port));

            //int32 MyInt = FCString::Atoi(*Remote_Port);
            Remote_Port_Short = FMath::Clamp<uint16>(Remote_Port_Int, 0, 65535);
            Remote_IP_Char = TCHAR_TO_UTF8(*Remote_IP);
            
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Port Unsigned Value: %hu"), Remote_Port_Short));
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("IP: %s"), ANSI_TO_TCHAR(Remote_IP_Char)));
            
        } else {
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("No Net Connection: %d"), i));
        }
        ++i;
    }

    TryStartGGPOSession();
}





void APoliticalGameStateBase::TryStartGGPOSession(){

    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("TryStartGGPOSession")));
    GGPOErrorCode result;
    GGPOSessionCallbacks cb = CreateCallBacks();

    const char Game_Name[] = "ThePoliticalUnderground";
    char* Game_Name_Pointer = (char*)Game_Name;
    //result = GGPONet::ggpo_start_session(&ggpo, &cb,  "ThePoliticalUnderground", 2, sizeof(uint8), 7777);
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Input Size: %d"), sizeof(uint8)));
    if(HasAuthority()){
        result = GGPONet::ggpo_start_session(&ggpo, &cb,  "ThePoliticalUnderground", 2, sizeof(int32), 2222);
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("UDP Start Port 7777")));  
    } else {
        result = GGPONet::ggpo_start_session(&ggpo, &cb,  "ThePoliticalUnderground", 2, sizeof(int32), 1111);
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("UDP Start Port 1111"))); 
    }
    
    
    
    

    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Result: %d"), (int32)result));
    if(GGPO_SUCCEEDED(result)){
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Start Session Success")));
    } else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Start Session Result: %d"), (int32)result));
    }
    /*
    result = GGPONet::ggpo_try_synchronize_local(ggpo);
    
    
    result = GGPONet::ggpo_start_synctest(&ggpo, &cb, Game_Name_Pointer, 2, sizeof(uint8), 1); // last param is frames

    if(GGPO_SUCCEEDED(result)){
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("SyncTest Success")));
    } else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Try Sync Local Result: %d"), (int32)result));
    }
    */
    
    
    GGPONet::ggpo_set_disconnect_timeout(ggpo, 3000);
    GGPONet::ggpo_set_disconnect_notify_start(ggpo, 1000);

    
    p1.size = p2.size = sizeof(GGPOPlayer);
    p1.player_num = 1;
    p1.type = EGGPOPlayerType::LOCAL;
    p2.player_num = 2;
    p2.type = EGGPOPlayerType::REMOTE; // remote player
    //strcpy_s(p2.u.remote.ip_address, "127.0.0.1");
    strcpy_s(p2.u.remote.ip_address, Remote_IP_Char);// ip addess of the player
    //p2.u.remote.port = Remote_Port_Short; 
    //p2.u.remote.port = 7777;   

    
    if(HasAuthority()){
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Remote Port 1111"))); 
        p2.u.remote.port = 1111; 
    } else {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Remote Port 7777"))); 
         p2.u.remote.port = 2222;
    }
    
    
    
         // port of that player

    result = GGPONet::ggpo_add_player(ggpo, &p1,  &player_handles[0]);
    if(GGPO_SUCCEEDED(result)){
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Add GGPOPlayer 1 Success")));
    } else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("GGPOPlayer 1 Result: %d"), (int32)result));
    }
    result = GGPONet::ggpo_add_player(ggpo, &p2,  &player_handles[1]);

    if(GGPO_SUCCEEDED(result)){
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Add GGPOPlayer 2 Success")));
    } else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("GGPOPlayer 2 Result: %d"), (int32)result));
    }

    
    GGPO_Started = true;
    int32 IdleMs = (int32)(ONE_FRAME - (int32)(ElapsedTime * 1000));
    PU_Idle(FMath::Max(0, IdleMs - 1));
    PU_RunFrame();
    Client_Character->SetGameTick(gTick);
    Connected_Character->SetGameTick(gTick);
}


void APoliticalGameStateBase::PU_RunFrame(){

    PU_GetNetworkStats(1);

    GGPOErrorCode result = GGPO_OK;
    int disconnect_flags;
    int inputs[MAX_PLAYERS];
    
    if(Client_Character->GetGameTick() == Client_Character->GetRollBack()){
        UE_LOG(LogTemp, Display, TEXT("GTick After Rollback: %d"), gTick);
    }

    Client_Character->SetGameTick(gTick);
    Connected_Character->SetGameTick(gTick);

    Client_Character->SetRollBackEnd(gTick);
    Connected_Character->SetRollBackEnd(gTick);
    //UE_LOG(LogTemp, Display, TEXT("Set Game Tick: %d"), gTick);

    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Calling Run Frame GS")));
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Local Input Size: %d"), sizeof(local_input)));
    result = GGPONet::ggpo_add_local_input(ggpo, player_handles[0], &local_input, sizeof(local_input));


    if (GGPO_SUCCEEDED(result)) {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Add Local Input Success!")));
        result = GGPONet::ggpo_synchronize_input(ggpo, (void*)inputs, sizeof(int) * MAX_PLAYERS, &disconnect_flags);
        if (GGPO_SUCCEEDED(result)) {
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Sync Input Success!")));
            // inputs[0] and inputs[1] contain the inputs for p1 and p2.  Advance
            // the game by 1 frame using those inputs.
            PU_AdvanceFrame(inputs, disconnect_flags, false);
        } else {
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Sync Result: %d"), (int32)result));

        }
    } else {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Add Local Input Result: %d"), (int32)result));
    }


    int size = sizeof(inputs);
    if(size > 1){
        if(inputs[0] != 0){
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("P1 Input: %d"), (int32)inputs[0]));
        }   
    }

    if(size > 1){
        if(inputs[1] != 32764){
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Retrieved P2 Input: %d"), (int32)inputs[1]));
        }
    }


}

void APoliticalGameStateBase::RetrieveLocalInputs_Implementation(uint8 input){
    if(input != 0){
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Local Input: %d"), (int32)input));
    }
    local_input = input;
}


    


void APoliticalGameStateBase::PU_AdvanceFrame(int32 inputs[], int32 disconnect_flags, bool IsRollback){
    // update the gamestate based on the inputs
    PU_GetNetworkStats(2);
    GGPOErrorCode result = GGPO_OK;
    if(!IsRollback){
        UpdateGameState(inputs, false);
    } else {
        UpdateGameState(inputs, true);
    }
    

    result = GGPONet::ggpo_advance_frame(ggpo);


    if (GGPO_SUCCEEDED(result)) {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Advance Frame Succeded: %d")));
    } else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Advance Frame Result: %d"), (int32)result));
    }
}


GGPOSessionCallbacks APoliticalGameStateBase::CreateCallBacks(){

    GGPOSessionCallbacks cb;


    cb.begin_game = std::bind(&APoliticalGameStateBase::pu_begin_game_callback, this, std::placeholders::_1);
    cb.save_game_state = std::bind(&APoliticalGameStateBase::pu_save_game_state_callback, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    cb.load_game_state = std::bind(&APoliticalGameStateBase::pu_load_game_state_callback, this,
        std::placeholders::_1, std::placeholders::_2);
    cb.log_game_state = std::bind(&APoliticalGameStateBase::pu_log_game_state, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    cb.free_buffer = std::bind(&APoliticalGameStateBase::pu_free_buffer, this, std::placeholders::_1);
    cb.advance_frame = std::bind(&APoliticalGameStateBase::pu_advance_frame_callback, this, std::placeholders::_1);
    cb.on_event = std::bind(&APoliticalGameStateBase::pu_on_event_callback, this, std::placeholders::_1);

   return cb;

}


bool APoliticalGameStateBase::pu_begin_game_callback(const char*){
    return true;
}


bool APoliticalGameStateBase::pu_save_game_state_callback(unsigned char** buffer, int32* len, int32* checksum, int32){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attempting to Save Game State")));


    GetCharacterStates(); // assigns the game state to the State of Both Players

    *len = sizeof(GS); // assigns *len or cbuf to len
    *buffer = (unsigned char*)malloc(*len); // allocated a dynamic block of memory to buffer of size len
    if (!*buffer) {
        return false;
    }
    memcpy(*buffer, &GS, *len); // copies gamestate to the buffer
    *checksum = fletcher32_checksum((short*)*buffer, *len / 2);
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("CheckSum: %d"), checksum));
    
    return true;

}


uint32_t APoliticalGameStateBase::fletcher32_checksum(const short* data, size_t len) {
    uint32_t sum1 = 0xffff, sum2 = 0xffff;

    while (len) {
        size_t tlen = len > 360 ? 360 : len;
        len -= tlen;
        do {
            sum2 += sum1 += *data++;
        } while (--tlen);

        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    // Finalization step
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);

    return (sum2 << 16) | sum1;
}


void APoliticalGameStateBase::SetClientCharacter_Implementation(APoliticalUndergroundCharacter* Character){
    if(Character->IsValidLowLevel()){
        //FString CharacterName = Character->GetName();
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Set Game State Client Character: %s"), *CharacterName));
        Client_Character = TStrongObjectPtr<APoliticalUndergroundCharacter>(Character);
        if(Client_Character){
            Client_Character_Simulation_Set = true;
        }

    } else {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Client Character in Game State Not Valid")));
    }
    
    
}



void APoliticalGameStateBase::SetConnectedCharacter_Implementation(APoliticalUndergroundCharacter* Character){
    if(Character->IsValidLowLevel()){
        //FString CharacterName = Character->GetName();
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Set Game State Client Character: %s"), *CharacterName));
        Connected_Character = TStrongObjectPtr<APoliticalUndergroundCharacter>(Character);
        if(Connected_Character){
            Connected_Character_Simulation_Set = true;
        }
    } else {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Connected Character in Game State Not Valid")));
    }
    
}



void APoliticalGameStateBase::GetCharacterStates(){

    Client_Character.Get()->SaveCharacterState(GS.PlayerOne);
	Connected_Character.Get()->SaveCharacterState(GS.PlayerTwo);

}


void APoliticalGameStateBase::UpdateGameState(int32 inputs[], bool rollback){
    int arraySize = sizeof(inputs) / sizeof(inputs[0]);
    if(arraySize != 2){
         GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Array Size: %d"), arraySize));
    }
    
    
    if(arraySize > 0){
        

        for(int i = 0; i < arraySize; ++i){


            if(i == 0 && rollback){
                //UE_LOG(LogTemp, Display, TEXT(""));
                //UE_LOG(LogTemp, Display, TEXT("Starting Rollback!!!"));
                Client_Character->CallRollBackTick();
            }

            if(i == 1 && rollback){
                //UE_LOG(LogTemp, Display, TEXT(""));
                //UE_LOG(LogTemp, Display, TEXT("Starting Rollback!!!"));
                Connected_Character->CallRollBackTick();
            }

            // Move Forward
            if(inputs[i] == 1){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->MoveRight(1.0, rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->MoveRight(1.0, rollback);
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Move Right Connected")));
                }
            }

            
            //Move Backward
            if(inputs[i] == 2){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->MoveRight(-1.0, rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->MoveRight(-1.0, rollback);
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Move Right Connected")));
                }
            }


             if(inputs[i] != 1 && inputs[i] != 2){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->MoveRight(0.0, rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->MoveRight(0.0, rollback);
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Move Right Connected")));
                }
            }

            // Up
            if(inputs[i] == 3){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->MoveForward(1.0, rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->MoveForward(1.0, rollback);
                }
            }


            // Down
            if(inputs[i] == 4){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->MoveForward(-1.0, rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->MoveForward(-1.0, rollback);
                }
            }


            // Attack1
            if(inputs[i] == 5){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->Attack1(rollback);
                    /*
                    UE_LOG(LogTemp, Display, TEXT(""));
                    UE_LOG(LogTemp, Display, TEXT("P1 Attack1"));
                    if(rollback){UE_LOG(LogTemp, Display, TEXT("Rollback"));}
                    UE_LOG(LogTemp, Display, TEXT("Tick: %d"), gTick);
                    UE_LOG(LogTemp, Display, TEXT("Game Tick: %d"), Client_Character->GetGameTick());
                    */
                    
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->Attack1(rollback);
                    /*
                    UE_LOG(LogTemp, Display, TEXT(""));
                    UE_LOG(LogTemp, Display, TEXT("P2 Attack1"));
                    if(rollback){UE_LOG(LogTemp, Display, TEXT("Rollback"));}
                    UE_LOG(LogTemp, Display, TEXT("Tick: %d"), gTick);
                    UE_LOG(LogTemp, Display, TEXT("Game Tick: %d"), Connected_Character->GetGameTick());
                    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack1")));
                    */
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack1 Connected")));
                }
            }


            // Attack2
            if(inputs[i] == 6){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->Attack2(rollback);
                    /*
                    UE_LOG(LogTemp, Display, TEXT(""));
                    UE_LOG(LogTemp, Display, TEXT("P1 Attack2"));
                    if(rollback){UE_LOG(LogTemp, Display, TEXT("Rollback"));}
                    UE_LOG(LogTemp, Display, TEXT("Tick: %d"), gTick);
                    UE_LOG(LogTemp, Display, TEXT("Game Tick: %d"), Client_Character->GetGameTick());
                    */
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack2")));
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->Attack2(rollback);
                    /*
                    UE_LOG(LogTemp, Display, TEXT(""));
                    UE_LOG(LogTemp, Display, TEXT("P2 Attack2"));
                    if(rollback){UE_LOG(LogTemp, Display, TEXT("Rollback"));}
                    UE_LOG(LogTemp, Display, TEXT("Tick: %d"), gTick);
                    UE_LOG(LogTemp, Display, TEXT("Game Tick: %d"), Connected_Character->GetGameTick());
                    */
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack2 Connected")));
                }
            }

            // Attack2
            if(inputs[i] == 7){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->Attack3(rollback);
                    /*
                    UE_LOG(LogTemp, Display, TEXT(""));
                    UE_LOG(LogTemp, Display, TEXT("P1 Attack3"));
                    if(rollback){UE_LOG(LogTemp, Display, TEXT("Rollback"));}
                    UE_LOG(LogTemp, Display, TEXT("Tick: %d"), gTick);
                    UE_LOG(LogTemp, Display, TEXT("Game Tick: %d"), Client_Character->GetGameTick());
                    */
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack3")));
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->Attack3(rollback);
                    /*
                    UE_LOG(LogTemp, Display, TEXT(""));
                    UE_LOG(LogTemp, Display, TEXT("P2 Attack3"));
                    if(rollback){UE_LOG(LogTemp, Display, TEXT("Rollback"));}
                    UE_LOG(LogTemp, Display, TEXT("Tick: %d"), gTick);
                    UE_LOG(LogTemp, Display, TEXT("Game Tick: %d"), Connected_Character->GetGameTick());
                    */
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack3 Connected")));
                }
            }


            // Attack3
            if(inputs[i] == 8){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->Attack4(rollback);
                    /*
                    UE_LOG(LogTemp, Display, TEXT(""));
                    UE_LOG(LogTemp, Display, TEXT("P1 Attack4"));
                    if(rollback){UE_LOG(LogTemp, Display, TEXT("Rollback"));}
                    UE_LOG(LogTemp, Display, TEXT("Tick: %d"), gTick);
                    UE_LOG(LogTemp, Display, TEXT("Game Tick: %d"), Client_Character->GetGameTick());
                    */
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack4")));
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->Attack4(rollback);
                    /*
                    UE_LOG(LogTemp, Display, TEXT(""));
                    UE_LOG(LogTemp, Display, TEXT("P2 Attack4"));
                    if(rollback){UE_LOG(LogTemp, Display, TEXT("Rollback"));}
                    UE_LOG(LogTemp, Display, TEXT("Tick: %d"), gTick);
                    UE_LOG(LogTemp, Display, TEXT("Game Tick: %d"), Connected_Character->GetGameTick());
                    */
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack4 Connected")));
                }
            }



            // Special Attack 1
            if(inputs[i] == 9){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->SpecialAttack1(rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->SpecialAttack1(rollback);
                }
            }

            // Special Attack 2
            if(inputs[i] == 10){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->SpecialAttack2(rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->SpecialAttack2(rollback);
                }
            }

            // Hold Pressed

            if(inputs[i] == 11){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->HoldPressed(rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->HoldPressed(rollback);
                }
            }

            // Hold Released

            if(inputs[i] == 12){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->HoldReleased(rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->HoldReleased(rollback);
                }
            }


            // Block 

            if(inputs[i] == 13){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->StartBlocking(rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->StartBlocking(rollback);
                }
            }


            // Stop Block 

            if(inputs[i] == 14){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->StopBlocking(rollback);
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->StopBlocking(rollback);
                }
            }

            // Jump
            if(inputs[i] == 15){
                if(Client_Character_Simulation_Set && i == 0){
                    Client_Character->Jump();
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    Connected_Character->Jump();
                }
            }

            //Sync
            if(inputs[i] == 18){
                if(Client_Character_Simulation_Set && i == 0){
                    //Client_Character->Sync();
                }
                if(Connected_Character_Simulation_Set && i == 1){
                    //Connected_Character->Sync();
                }
            }

            
        }

    

    } else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Empty Array!")));
    }

}

bool APoliticalGameStateBase::pu_load_game_state_callback(unsigned char* buffer, int32 len){

    //UE_LOG(LogTemp, Display, TEXT("Set End Rollback Tick: %d"), gTick);

    memcpy(&GS, buffer, len);
    Client_Character.Get()->LoadCharacterState(GS.PlayerOne);
	Connected_Character.Get()->LoadCharacterState(GS.PlayerTwo);

    // copy to the characters
    
    return true;
}
bool APoliticalGameStateBase::pu_log_game_state(char* filename, unsigned char* buffer, int32){
    return true;

}
void APoliticalGameStateBase::pu_free_buffer(void* buffer){
    free(buffer);
}

bool APoliticalGameStateBase::pu_advance_frame_callback(int32){


    int disconnect_flags;
    int inputs[MAX_PLAYERS];
    GGPOErrorCode result = GGPO_OK;

    // Make sure we fetch new inputs from GGPO and use those to update
    // the game state instead of reading from the keyboard.

    
    result = GGPONet::ggpo_synchronize_input(ggpo, (void*)inputs, sizeof(int) * MAX_PLAYERS, &disconnect_flags);



    if(GGPO_SUCCEEDED(result)) {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Advance Frame Succeded: %d")));
        PU_AdvanceFrame(inputs, disconnect_flags, true);
        return true;
    } else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Advance Frame Result: %d"), (int32)result));
        return false;
    }
    

}

bool APoliticalGameStateBase::pu_on_event_callback(GGPOEvent* info){

    int progress;
    switch (info->code) {
    case GGPO_EVENTCODE_CONNECTED_TO_PEER:
        //ngs.SetConnectState(info->u.connected.player, EPlayerConnectState::Synchronizing);
        break;
    case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
        progress = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
        break;
    case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
        //ngs.UpdateConnectProgress(info->u.synchronized.player, 100);
        break;
    case GGPO_EVENTCODE_RUNNING:
        break;
    case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
        //ngs.SetDisconnectTimeout(info->u.connection_interrupted.player,
            //get_time(),
            //info->u.connection_interrupted.disconnect_timeout);
        break;
    case GGPO_EVENTCODE_CONNECTION_RESUMED:
        //ngs.SetConnectState(info->u.connection_resumed.player, EPlayerConnectState::Running);
        break;
    case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
        //ngs.SetConnectState(info->u.disconnected.player, EPlayerConnectState::Disconnected);
        break;
    case GGPO_EVENTCODE_TIMESYNC:
        //Sleep(1000 * info->u.timesync.frames_ahead / 60);
        break;
    }
    return true;

}


void APoliticalGameStateBase::PU_GetNetworkStats(int type)
{
    FGGPONetworkStats Stats;
    GGPONet::ggpo_get_network_stats(ggpo, player_handles[1], &Stats);
    int32 LocalFairness = Stats.timesync.local_frames_behind;
    int32 RemoteFairness = Stats.timesync.remote_frames_behind;
    int32 Fairness;
    int32 Ping = Stats.network.ping;


    float FrameTime = GetWorld()->GetDeltaSeconds();
    
    // Convert frame time to milliseconds
    float FrameTimeMS = FrameTime * 1000;
    int32 PingFrames = FMath::RoundToInt(Ping/FrameTimeMS);
    

    if (LocalFairness < 0 && RemoteFairness < 0) {
            /*
             * Both think it's unfair (which, ironically, is fair).  Scale both and subtrace.
             */
            Fairness = abs(abs(LocalFairness) - abs(RemoteFairness));
    }
    else if (LocalFairness > 0 && RemoteFairness > 0) {
        /*
            * Impossible!  Unless the network has negative transmit time.  Odd....
            */
        Fairness = 0;
    }
    else {
        /*
            * They disagree.  Add.
            */
        Fairness = -1*(LocalFairness + RemoteFairness);
    }


    Client_Character->SetFairness(Fairness);
    Connected_Character->SetFairness(Fairness);

    
    
    //UE_LOG(LogTemp, Display, TEXT("GTick: %d Fairness: %d"), gTick, Fairness);
  
    
    
}





void APoliticalGameStateBase::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);
        //Replicate current health.
        //DOREPLIFETIME(APoliticalGameStateBase, gTick);       
        
}