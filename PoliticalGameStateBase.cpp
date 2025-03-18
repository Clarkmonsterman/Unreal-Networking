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
    //GetConnectedPlayerNetworkInfo_Implementation();
}




void APoliticalGameStateBase::GetConnectedPlayerNetworkInfo_Implementation(int Port, const FString& IP) {

   
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Get Network Info")));
   
    if(HasAuthority()){

   

        if (GetWorld() && GetWorld()->GetNetDriver())
        {
            for (UNetConnection* Connection : GetWorld()->GetNetDriver()->ClientConnections)
            {
                if (Connection && Connection->RemoteAddr.IsValid())
                {
                    FString ClientIP = Connection->RemoteAddr->ToString(true); // true = exclude port
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Authority")));
                    ClientIP.Split(TEXT(":"), &Remote_IP, &Remote_Port);
                    Remote_IP_Char = TCHAR_TO_UTF8(*Remote_IP);
                    // Convert Port (as FString) to unsigned short
                    Remote_Port_Short = static_cast<unsigned short>(FCString::Atoi(*Remote_Port));
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Remote_IP_Char: %s, Remote_Port_Short: %d"), *FString(Remote_IP_Char), Remote_Port_Short));

                            
                }  
            }
        } else {
            UE_LOG(LogTemp, Warning, TEXT("No NetDriver or invalid World."));
        }

    } else {

        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Client")));
        Remote_IP_Char = TCHAR_TO_UTF8(*IP);
        // Convert Port to unsigned short
        Remote_Port_Short = static_cast<unsigned short>(Port);
        // Debug or use the converted variables
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Remote_IP_Char: %s, Remote_Port_Short: %d"), *FString(Remote_IP_Char), Remote_Port_Short));

    
    }
    
    TryStartGGPOSession();
}





void APoliticalGameStateBase::TryStartGGPOSession(){

    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("TryStartGGPOSession")));
    GGPOErrorCode result;
    GGPOSessionCallbacks cb = CreateCallBacks();

    const char Game_Name[] = "ThePoliticalUnderground";
    char* Game_Name_Pointer = (char*)Game_Name;
    //result = GGPONet::ggpo_start_session(&ggpo, &cb,  "ThePoliticalUnderground", 2, sizeof(uint8), 7777);
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Input Size: %d"), sizeof(uint8)));

    const int32 LocalPort = HasAuthority() ? 8001 : 8002;
    const int32 RemotePort = HasAuthority() ? 8002 : 8001;

    result = GGPONet::ggpo_start_session(&ggpo, &cb, "ThePoliticalUnderground", 2, sizeof(int32), LocalPort);
    if(GGPO_SUCCEEDED(result)){
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Start Session Success")));
    } else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Start Session Result: %d"), (int32)result));
    }


 
    
    GGPONet::ggpo_set_disconnect_timeout(ggpo, 3000);
    GGPONet::ggpo_set_disconnect_notify_start(ggpo, 1000);

    
    p1.size = p2.size = sizeof(GGPOPlayer);
    p1.player_num = 1;
    p1.type = EGGPOPlayerType::LOCAL;
    p2.player_num = 2;
    p2.type = EGGPOPlayerType::REMOTE; // remote player
    //strcpy_s(p2.u.remote.ip_address, "127.0.0.1");
    strcpy_s(p2.u.remote.ip_address, Remote_IP_Char);// ip addess of the player
    p2.u.remote.port = RemotePort;

    
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
    

    Client_Character->SetGameTick(gTick);
    Connected_Character->SetGameTick(gTick);

    Client_Character->SetRollBackEnd(gTick);
    Connected_Character->SetRollBackEnd(gTick);
    //UE_LOG(LogTemp, Display, TEXT("Set Game Tick: %d"), gTick);

    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Calling Run Frame GS")));
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Local Input Size: %d"), sizeof(local_input)));
    if(local_input != 0){
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Local Input: %d"), (int32)local_input));

    }

    result = GGPONet::ggpo_add_local_input(ggpo, player_handles[0], &local_input, sizeof(local_input));


    if (GGPO_SUCCEEDED(result)) {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Add Local Input Success!")));
        result = GGPONet::ggpo_synchronize_input(ggpo, (void*)inputs, sizeof(int) * MAX_PLAYERS, &disconnect_flags);
        if (GGPO_SUCCEEDED(result)) {
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Sync Input Success!")));

            int size = sizeof(inputs);
            if(size > 1){
                if(inputs[0] != 0){
                    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("P1 Synced Input: %d"), (int32)inputs[0]));
                }   
            }

            if(size > 1){
                if(inputs[1] != 32764){
                   // GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Retrieved P2 Input: %d"), (int32)inputs[1]));
                }
            }
                    // inputs[0] and inputs[1] contain the inputs for p1 and p2.  Advance
            // the game by 1 frame using those inputs.
            PU_AdvanceFrame(inputs, disconnect_flags, false);
        } else {
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Sync Result: %d"), (int32)result));

        }
    } else {
       //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Add Local Input Result: %d"), (int32)result));
    }


    


}

void APoliticalGameStateBase::RetrieveLocalInputs_Implementation(int32 input){
    if(input != 0){
        float test = input / 10.0f;
        if(test >= 10){
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Input Received: %d"), input));
        }
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

    
    
    if(arraySize > 0){
        

        for(int i = 0; i < arraySize; ++i){

           

            int32 Value = inputs[i];
            TArray<int32> InputValues;
    

            FString NumberString = FString::Printf(TEXT("%d"), Value);
            int32 Length = NumberString.Len();

            for (int32 z = 0; z < Length; z += 2) {
                FString Val = NumberString.Mid(z, FMath::Min(2, Length - z)); // Get 2-digit substring
                InputValues.Add(FCString::Atoi(*Val)); // Convert to int and add to array
            }

            if(InputValues.Num() > 1){
                GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Update Multiple Inputs: %d"), inputs[i]));
            }

            for(int32 Input : InputValues){


                if(InputValues.Num() > 1){
                    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Input: %d"), Input));
                }

                if(Input == 10){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->MoveRight(0.0f, rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->MoveRight(0.0f, rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Move Right Connected")));
                    }


                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->MoveForward(0.0f, rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->MoveForward(0.0f, rollback);
                    }
                }

                // Move Forward
                if(Input == 11){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->MoveRight(1.0f, rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->MoveRight(1.0f, rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Move Right Connected")));
                    }
                }

                
                //Move Backward
                if(Input == 12){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->MoveRight(-1.0f, rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->MoveRight(-1.0f, rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Move Right Connected")));
                    }
                }


                // Up
                if(Input == 13){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->MoveForward(1.0f, rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->MoveForward(1.0f, rollback);
                    }
                }


                // Down
                if(Input == 14){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->MoveForward(-1.0f, rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->MoveForward(-1.0f, rollback);
                    }
                }


                // Attack1
                if(Input == 15){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->Attack1(rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->Attack1(rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack1 Connected")));
                    }
                }


                // Attack2
                if(Input == 16){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->Attack2(rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack2")));
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->Attack2(rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack2 Connected")));
                    }
                }

                // Attack2
                if(Input == 17){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->Attack3(rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack3")));
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->Attack3(rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack3 Connected")));
                    }
                }


                // Attack3
                if(Input == 18){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->Attack4(rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack4")));
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->Attack4(rollback);
                        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Attack4 Connected")));
                    }
                }



                // Special Attack 1
                if(Input == 19){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->SpecialAttack1(rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->SpecialAttack1(rollback);
                    }
                }

                // Special Attack 2
                if(Input == 20){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->SpecialAttack2(rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->SpecialAttack2(rollback);
                    }
                }

                // Hold Pressed

                if(Input == 21){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->HoldPressed(rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->HoldPressed(rollback);
                    }
                }

                // Hold Released

                if(Input == 22){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->HoldReleased(rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->HoldReleased(rollback);
                    }
                }


                // Block 

                if(Input == 23){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->StartBlocking(rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->StartBlocking(rollback);
                    }
                }


                // Stop Block 

                if(Input == 24){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->StopBlocking(rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->StopBlocking(rollback);
                    }
                }

                // Jump
                if(Input == 25){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->CustomJump(rollback);
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->CustomJump(rollback);
                    }
                }


                if(Input == 26){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->ReleaseAttack1();
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->ReleaseAttack1();
                    }
                }


                if(Input == 27){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->ReleaseAttack2();
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->ReleaseAttack2();
                    }
                }


                if(Input == 28){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->ReleaseAttack3();
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->ReleaseAttack3();
                    }
                }


                if(Input == 29){
                    if(Client_Character_Simulation_Set && i == 0){
                        Client_Character->ReleaseAttack4();
                    }
                    if(Connected_Character_Simulation_Set && i == 1){
                        Connected_Character->ReleaseAttack4();
                    }
                }
            }

            if(i == 0 && rollback){
                Client_Character->CharacterTickLogic(true);
            }

            if(i == 1 && rollback){
                Connected_Character->CharacterTickLogic(true);
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

    
    //UE_LOG(LogTemp, Display, TEXT("GTick: %d Fairness: %d"), gTick, Fairness);
  
    
    
}






void APoliticalGameStateBase::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);
        //Replicate current health.
        //DOREPLIFETIME(APoliticalGameStateBase, gTick);       
        
}