// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerController.h"

// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/GameplayStatics.h"
#include "Misc/CoreMiscDefines.h"
#include "BaseGameInstance.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine.h"
#include "EngineUtils.h" // Include the header for TActorIterator
#include "CoreMinimal.h"

#include "Net/UnrealNetwork.h"
#include "BaseGameInstance.h"



ABasePlayerController::ABasePlayerController(){
    //UE_LOG(LogTemp, Display, TEXT("Starting Player Controller"));
    bAutoManageActiveCameraTarget = false;
    MenuSelectFrame = -1;
    PlatFormUser_ID = -1;
    LocalPlayer_Id = -1;

    Simulator_Character_Set = false;
    IsNetwork_Player = false;
    GameStateCast = false;
    CharacterCastToGameState = false;
    controllerTicks = 0;
    postSpawnTicks = -1;
    /* Initialize The Values */
	

}



void ABasePlayerController::BeginPlay(){
    Super::BeginPlay();
    bool local_control = IsLocalController();
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Starting Player Controller")));
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Local Control? %s"), local_control ? TEXT("True") : TEXT("False")));;

   //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Start Controller")));

    

    if(auto gameMode = Cast<APoliticalUndergroundGameMode>(GetWorld()->GetAuthGameMode())){
        ABasePlayerController* Self = this;
        if(local_control){
             gameMode->AddBaseController(this); 
        }
	}

    //UE_LOG(LogTemp, Warning, TEXT("Starting Player Controller"));
    if(GetLocalPlayer() != nullptr){
        LocalPlayer_Id = GetLocalPlayer()->GetControllerId();
        PlatFormUser_ID = GetLocalPlayer()->GetPlatformUserId();
        UE_LOG(LogTemp, Warning, TEXT("LocalPlayer ID: %d"), LocalPlayer_Id);
        UE_LOG(LogTemp, Display, TEXT("PlatFormID: %d"), PlatFormUser_ID);

        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("LocalPlayer ID: %d"), LocalPlayer_Id));
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("PlatFormID: %d"), PlatFormUser_ID));
    } else {
        LocalPlayer_Id = -10;
    }



    if (NetConnection) // This is a field of PlayerController
    {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Net request url is %s"), *NetConnection->URL.Host));
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Net request port is %d"), NetConnection->URL.Port));
    }
    else
    {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("NetConnection is null")));
    }

    // Retrieve the initially possessed pawn.
    
}






void ABasePlayerController::Tick(float DeltaTime){
    Super::Tick(DeltaTime);

    if(GetLocalPlayer() != nullptr){
        LocalPlayer_Id = GetLocalPlayer()->GetControllerId();
        PlatFormUser_ID = GetLocalPlayer()->GetPlatformUserId();
        //UE_LOG(LogTemp, Warning, TEXT("Controller ID: %d"), LocalPlayer_Id);
        //UE_LOG(LogTemp, Display, TEXT("PlatFormID: %d"), PlatFormUser_ID);
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("LocalPlayer ID: %d"), LocalPlayer_Id));
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("PlatFormID: %d"), PlatFormUser_ID));
    } else {
        LocalPlayer_Id = -10;
        //UE_LOG(LogTemp, Warning, TEXT("Controller ID: %d"), Id);
    }
    
    
    if(MenuSelectFrame > 0){
        --MenuSelectFrame;
        SetMenuInputs();
    } else if(MenuSelectFrame == 0){
        //UE_LOG(LogTemp, Display, TEXT("Reset Menu Select"));
        MenuPress = EMenuType::VE_None;
        SetMenuInputs();
        --MenuSelectFrame;
    }

    

    if(!GameStateCast && Simulator_Character_Set){
        NetworkGameStateCast_Attempt();
    }
    
    if(postSpawnTicks >= 0){
        postSpawnTicks++;
        if(IsLocalController()){
            if(postSpawnTicks == 1){
                SpawnGameCamera();
            }
            if(postSpawnTicks == 120){
                Player_Two.Get()->StartFight();
                Player_One.Get()->StartFight();
                
            }
            
        }
    }
    

    controllerTicks++;

    if(GameStateCast && IsNetwork_Player){
        
        if(ActionSelectFrame > 0){
            SendLocalInputs();
            ActionSelectFrame = 0;
        } else if(ActionSelectFrame == 0){
            convertInputsPressed(10);
            SendLocalInputs();
        }
    }
    

}

void ABasePlayerController::SetupInputComponent(){
    Super::SetupInputComponent();
    
	InputComponent->BindAction("Jump", IE_Pressed, this, &ABasePlayerController::CallJump);
    InputComponent->BindAxis("Move Right / Left", this, &ABasePlayerController::CallMoveRight);
    InputComponent->BindAxis("Move Forward / Backward", this, &ABasePlayerController::CallMoveForward);
    InputComponent->BindAction("Block", IE_Pressed, this, &ABasePlayerController::CallStartBlocking);
    InputComponent->BindAction("Block", IE_Released, this, &ABasePlayerController::CallStopBlocking);
    InputComponent->BindAction("Attack1", IE_Pressed, this, &ABasePlayerController::CallAttack1);
    InputComponent->BindAction("Attack1", IE_Released, this, &ABasePlayerController::ReleaseAttack1);
    InputComponent->BindAction("Attack2", IE_Pressed, this, &ABasePlayerController::CallAttack2);
    InputComponent->BindAction("Attack2", IE_Released, this, &ABasePlayerController::ReleaseAttack2);
    InputComponent->BindAction("Attack3", IE_Pressed, this, &ABasePlayerController::CallAttack3);
    InputComponent->BindAction("Attack3", IE_Released, this, &ABasePlayerController::ReleaseAttack3);
    InputComponent->BindAction("Attack4", IE_Pressed, this, &ABasePlayerController::CallAttack4);
    InputComponent->BindAction("Attack4", IE_Released, this, &ABasePlayerController::ReleaseAttack4);
    InputComponent->BindAction("SpecialAttack1", IE_Pressed, this, &ABasePlayerController::CallSpecialAttack1);
    InputComponent->BindAction("SpecialAttack2", IE_Pressed, this, &ABasePlayerController::CallSpecialAttack2);
    InputComponent->BindAction("Hold", IE_Pressed, this, &ABasePlayerController::CallHoldPressed);
    InputComponent->BindAction("Hold", IE_Released, this, &ABasePlayerController::CallHoldReleased);
    InputComponent->BindAction("AnyKey", IE_Released, this, &ABasePlayerController::DetermineInputDeviceDetails);

   


    FInputActionBinding& anyKeyBinding = InputComponent->BindAction("AnyKey", IE_Pressed, this, &ABasePlayerController::DetermineInputDeviceDetails);
    anyKeyBinding.bConsumeInput = false;
    

    for(int i = 0; i <InputComponent->GetNumActionBindings(); ++i){
        InputComponent->GetActionBinding(i).bConsumeInput = false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Controller ID: %d"), LocalPlayer_Id);
    UE_LOG(LogTemp, Display, TEXT("PlatFormID: %d"), PlatFormUser_ID);

    if(LocalPlayer_Id == -1){
        UE_LOG(LogTemp, Display, TEXT("Bindind P1 Menu Controls"));
        InputComponent->BindAction("MenuLeftP1", IE_Pressed, this, &ABasePlayerController::CallMenuLeft);
        InputComponent->BindAction("MenuRightP1", IE_Pressed, this, &ABasePlayerController::CallMenuRight);
        InputComponent->BindAction("MenuDownP1", IE_Pressed, this, &ABasePlayerController::CallMenuDown);
        InputComponent->BindAction("MenuUpP1", IE_Pressed, this, &ABasePlayerController::CallMenuUp);
        InputComponent->BindAction("MenuConfirmP1", IE_Pressed, this, &ABasePlayerController::CallMenuConfirm);
        InputComponent->BindAction("MenuBackP1", IE_Pressed, this, &ABasePlayerController::CallMenuBack);
    } else {
        UE_LOG(LogTemp, Display, TEXT("Bindind P2 Menu Controls"));
        InputComponent->BindAction("MenuLeftP2", IE_Pressed, this, &ABasePlayerController::CallMenuLeft);
        InputComponent->BindAction("MenuRightP2", IE_Pressed, this, &ABasePlayerController::CallMenuRight);
        InputComponent->BindAction("MenuDownP2", IE_Pressed, this, &ABasePlayerController::CallMenuDown);
        InputComponent->BindAction("MenuUpP2", IE_Pressed, this, &ABasePlayerController::CallMenuUp);
        InputComponent->BindAction("MenuConfirmP2", IE_Pressed, this, &ABasePlayerController::CallMenuConfirm);
        InputComponent->BindAction("MenuBackP2", IE_Pressed, this, &ABasePlayerController::CallMenuBack);
    }
}

void ABasePlayerController::ResetMenuSelection(){
    MenuPress = EMenuType::VE_None;
}

void ABasePlayerController::CallMenuRight(){
    MenuPress = EMenuType::VE_Right;
    MenuSelectFrame = 1;
    //UE_LOG(LogTemp, Display, TEXT("Call Menu Right"));
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Menu Right")));
}
void ABasePlayerController::CallMenuLeft(){
    MenuPress = EMenuType::VE_Left;
    MenuSelectFrame = 1;
    //UE_LOG(LogTemp, Display, TEXT("Call Menu Left"));
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Menu Left")));
}
void ABasePlayerController::CallMenuUp(){
    MenuPress = EMenuType::VE_Up;
    MenuSelectFrame = 1;
    //UE_LOG(LogTemp, Display, TEXT("Call Menu Up"));
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Menu Up")));
}
void ABasePlayerController::CallMenuDown(){
    MenuPress = EMenuType::VE_Down;
    MenuSelectFrame = 1;
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Menu Down")));
    //UE_LOG(LogTemp, Display, TEXT("Call Menu Down"));
}

void ABasePlayerController::CallMenuConfirm(){
    MenuPress = EMenuType::VE_Confirm;
    MenuSelectFrame = 1;
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Menu Confirm")));
   // UE_LOG(LogTemp, Display, TEXT("Call Menu Confirm"));
}


void ABasePlayerController::CallMenuBack(){
    MenuPress = EMenuType::VE_Back;
    MenuSelectFrame = 1;
    //UE_LOG(LogTemp, Display, TEXT("Call Menu Back"));
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Menu Back")));
}

void ABasePlayerController::PossessCharacter(APawn* CharacterToPosess){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Calling Character To Possess")));
    Possess(CharacterToPosess);
    
}


void ABasePlayerController::DetermineInputDeviceDetails(FKey _keyPressed){
    //UE_LOG(LogTemp, Display, TEXT("FKey: %s"), *_keyPressed.ToString());
    // UE_LOG(LogTemp, Display, TEXT("PlatForm ID: %d"), PlatFormUser_ID);
    //UE_LOG(LogTemp, Display, TEXT("LocalPlayer ID: %d"), LocalPlayer_Id);
   
    if(!nullController){
        if(_keyPressed.IsGamepadKey())
        {
            //UE_LOG(LogTemp, Display, TEXT("GamePadKey Pressed"));
            isInputDeviceGamepad = true;
        } else {
            //UE_LOG(LogTemp, Display, TEXT("GamePadKey Not Pressed"));
            isInputDeviceGamepad = false;
        }

        //UE_LOG(LogTemp, Display, TEXT("UseGamePad: %s"), possessedPawn->UseGamePad? TEXT("True") : TEXT("False"));
    }

    
}



void ABasePlayerController::CallMoveRight(float _value){

    if(_value > 0 && ActionSelectFrame != 1){
        convertInputsPressed(11);
        ActionSelectFrame = 1;
    } else if(_value < 0 && ActionSelectFrame != 1){
        convertInputsPressed(12);
        ActionSelectFrame = 1;
    }

    
}


void ABasePlayerController::CallMoveForward(float _value){
     
    
    
    if(_value > 0 && ActionSelectFrame != 1){
        convertInputsPressed(13);
        ActionSelectFrame = 1;
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Calling Up From Controller")));
    } else if(_value < 0 && ActionSelectFrame != 1) {
        convertInputsPressed(14);
        ActionSelectFrame = 1;
    }

    
}



void ABasePlayerController::CallAttack1(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Attack1")));
    convertInputsPressed(15);
    ActionSelectFrame = 1;
}



void ABasePlayerController::CallAttack2(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Attack2")));
    convertInputsPressed(16);
    ActionSelectFrame = 1;
}



void ABasePlayerController::CallAttack3(){
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Attack2")));
    convertInputsPressed(17);
    ActionSelectFrame = 1;
}





void ABasePlayerController::CallAttack4(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Attack4")));
    convertInputsPressed(18);
    ActionSelectFrame = 1;
    
}


void ABasePlayerController::CallSpecialAttack1(){

    convertInputsPressed(19);
    ActionSelectFrame = 1;
}
void ABasePlayerController::CallSpecialAttack2(){

    convertInputsPressed(20);
    ActionSelectFrame = 1;
}

void ABasePlayerController::CallHoldPressed(){

    convertInputsPressed(21);
    ActionSelectFrame = 1;
}
void ABasePlayerController::CallHoldReleased(){

    convertInputsPressed(22);
    ActionSelectFrame = 1;
}



void ABasePlayerController::CallStartBlocking(){
   convertInputsPressed(23);
    ActionSelectFrame = 1;
}

void ABasePlayerController::CallStopBlocking(){

    convertInputsPressed(24);
    ActionSelectFrame = 1;
}


void ABasePlayerController::CallJump(){
    //UE_LOG(LogTemp, Display, TEXT("Call Jump: %d"), postSpawnTicks);
   convertInputsPressed(25);
   ActionSelectFrame = 1;
}


void ABasePlayerController::ReleaseAttack1(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Release Attack1")));
    convertInputsPressed(26);
    ActionSelectFrame = 1;
   
}

void ABasePlayerController::ReleaseAttack2(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Release Attack2")));
    convertInputsPressed(27);
    ActionSelectFrame = 1;
   
}

void ABasePlayerController::ReleaseAttack3(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Release Attack3")));
    convertInputsPressed(28);
    ActionSelectFrame = 1;
   
}


void ABasePlayerController::ReleaseAttack4(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Release Attack4")));
    convertInputsPressed(29);
    ActionSelectFrame = 1;
   
}



void ABasePlayerController::convertInputsPressed(int32 input){
    if(inputPressed != -1){
        inputPressed = inputPressed*100 + input;
    } else {
        inputPressed = input;
    }
}


void ABasePlayerController::SendLocalInputs() {
   
    


    if(ActionSelectFrame != 0){
        float test = inputPressed / 10.0f;
        if(test >= 10){
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Input Sent: %d"), inputPressed));
        }
    } else {
       // GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Zero Input Sent: %d"), inputPressed));
    }

    
    if (GameState) {
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Input Sent: %d"), finalValue));
        GameState->RetrieveLocalInputs_Implementation(inputPressed);
    }
    inputPressed = -1;

}


void ABasePlayerController::SetMenuInputs(){
   
    if(MenuPress != EMenuType::VE_None){
        MoveSelection();
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Total Rows: %d"), Total_Rows));
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Total Columns: %d"), Total_Columns));
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Current Row: %d"), Row));
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Current Column: %d"), Column));
        if(Column != 0){
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Row Init: %d"), Row_Init));
            

        }
        

    } else {
        MenuMoved = false;
    }
}


void ABasePlayerController::NetworkGameStateCast_Attempt(){
    
    GameState = Cast<APoliticalGameStateBase>(GetWorld()->GetGameState());

    int Remote_Port_Int = 0;
    FString Remote_IP;

    if(GameState){
        GameState->SetClientCharacter_Implementation(Weak_PlayerOne);
        GameState->SetConnectedCharacter_Implementation(Weak_PlayerTwo); 

        if(!HasAuthority()){

            UNetConnection* NetConnection_Info = GetNetConnection();
            
            if (!NetConnection_Info)
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("NetConnection is null!")));
                UE_LOG(LogTemp, Error, TEXT("NetConnection is null!"));
                return;
            }

            if (NetConnection_Info->URL.Host.IsEmpty())
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("NetConnection URL Host is empty!")));

                UE_LOG(LogTemp, Error, TEXT("NetConnection URL Host is empty!"));
                return;
            }

            Remote_Port_Int = NetConnection_Info->URL.Port;
            Remote_IP = *NetConnection_Info->URL.Host;

        
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Port: %d"), Remote_Port_Int));
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("IP: %s"), *Remote_IP));
            GameState->GetConnectedPlayerNetworkInfo_Implementation(Remote_Port_Int, Remote_IP);
    
        } else {
            GameState->GetConnectedPlayerNetworkInfo_Implementation(Remote_Port_Int, Remote_IP);

        }

        GameStateCast = true;
    } else {
        GameStateCast = false;
    }

}

 


void ABasePlayerController::Update_Status_Implementation(bool status, int PlayerNum){

    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Update Player %d"), PlayerNum));
    IsNetwork_Player = status;
    PlayerNumber = PlayerNum;
    if(IsNetwork_Player){
        SpawnCharactersLocal_Implementation();
    }
    
}


void ABasePlayerController::SpawnCharacter(FVector SpawnLoc, FRotator SpawnRot, int _player){
    // Spawn the character
                
        UObject* SpawnActor = Cast<UObject>(StaticLoadObject(UObject::StaticClass(), NULL, TEXT("/Game/ThirdPerson/Blueprints/BP_Tremp.BP_Tremp")));
        //UObject* SpawnActor = Cast<UObject>(StaticLoadObject(UObject::StaticClass(), NULL, TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPerson.BP_ThirdPerson")));

        if (SpawnActor)
        {
            UBlueprint* GeneratedBP = Cast<UBlueprint>(SpawnActor);
            if (GeneratedBP)
            {
                UClass* GeneratedClass = GeneratedBP->GeneratedClass;
                if(IsLocalController()){
                    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(GeneratedClass, SpawnLoc, SpawnRot);
                    if (SpawnedActor)
                    {
                        //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Spawned!!!")));
                        if(_player == 1){
                            //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("P1!!!")));
                            Weak_PlayerOne = Cast<APoliticalUndergroundCharacter>(SpawnedActor);
                            Player_One = TStrongObjectPtr<APoliticalUndergroundCharacter>(Weak_PlayerOne);
                            if(Player_One){
                                //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("PU Cast Successful")));
                            } else {
                                //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("PU Cast Failed")));
                            }
                        }
                        if(_player == 2){
                            //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("P2!!!")));
                            Weak_PlayerTwo = Cast<APoliticalUndergroundCharacter>(SpawnedActor);
                            Player_Two = TStrongObjectPtr<APoliticalUndergroundCharacter>(Weak_PlayerTwo);
                            if(Player_Two){
                                //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("PU Cast Successful")));
                            } else {
                                //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("PU Cast Failed")));
                            }
                        }
                        
                    }
                    else
                    {
                        //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Failed to Spawn Actor!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")));
                    }
                }

                
            } else {
                //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Failed to Cast to Blueprint")));
            }

        } else {
            //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("CANT FIND OBJECT TO SPAWN")));
        }
}

void ABasePlayerController::SpawnCharactersLocal_Implementation(){

    UWorld* World = GetWorld();
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Spawning Characters")));

    if (World)
    {
        // Iterate over all PlayerStart objects in the level
        for (TActorIterator<APlayerStart> It(World); It; ++It)
        {
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Player Start")));
            APlayerStart* PlayerStart = *It;
            bool isP1 = false;
            bool isP2 = false;

            if(PlayerStart->PlayerStartTag.ToString().Contains("P1")){
                isP1 = true;
            }

            if(PlayerStart->PlayerStartTag.ToString().Contains("P2")){
                isP2 = true;
            }
            
            if (PlayerStart)
            {
                FVector SpawnLoc = PlayerStart->GetActorLocation();
                SpawnLoc.Z = 0;
                FRotator SpawnRot= PlayerStart->GetActorRotation();
                
                //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Character Spawn Position: %s"), *SpawnLoc.ToString()));
                //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Character Spawn Rotation: %s"), *SpawnRot.ToString()));

                
                if(PlayerNumber == 1){
                    if(isP1){
                        SpawnCharacter(SpawnLoc, SpawnRot, 1);
                    } else {
                        SpawnCharacter(SpawnLoc, SpawnRot, 2);
                    }
                }

                if(PlayerNumber == 2){
                    if(isP1){
                        SpawnCharacter(SpawnLoc, SpawnRot, 2);
                    } else {
                        SpawnCharacter(SpawnLoc, SpawnRot, 1);
                    }
                }
                
    
                
                //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Spawn Location: %s"), *SpawnLoc.ToString()));
                //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Spawn Rotation: %s"), *SpawnRot.ToString()));
                
            
            } else {
                //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("No Player Start")));
            }
        }
    }

    if(Player_One.Get()->IsValidLowLevel() && Player_Two.Get()->IsValidLowLevel()){
        Player_One.Get()->SetOtherPlayer();
        Player_One.Get()->CharacterPlayerNum = 1;
        Player_Two.Get()->SetOtherPlayer();
        Player_Two.Get()->CharacterPlayerNum = 2;
       
    } else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Failed Character Validity Check")));
    }
    
    Simulator_Character_Set = true;
    postSpawnTicks = 0;
   
}

void ABasePlayerController::SpawnGameCamera(){


        UObject* SpawnCamera = Cast<UObject>(StaticLoadObject(UObject::StaticClass(), NULL, TEXT("/Game/Cameras/BP_NetworkCamera.BP_NetworkCamera")));
        if (SpawnCamera)
        {
            UBlueprint* GeneratedBP = Cast<UBlueprint>(SpawnCamera);
            if (GeneratedBP)
            {
                UClass* GeneratedClass = GeneratedBP->GeneratedClass;
                FVector SpawnLoc = FVector(0.0f, 0.0f, 0.0f);
                FRotator SpawnRot = FRotator(0.0f, 0.0f, 0.0f);
                if(IsLocalController()){
                    AActor* CameraActor = GetWorld()->SpawnActor<AActor>(GeneratedClass, SpawnLoc, SpawnRot);
                    SetViewTargetWithBlend(CameraActor, 0.5f);
                }
            }
        } else {
             GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Can't Find Network Camera")));
        }
}






void ABasePlayerController::MoveSelection()
{
    if(!Continous){
   
        switch (MenuPress)
        {
            case EMenuType::VE_None:
                // Handle the 'None' case
                break;

            case EMenuType::VE_Up:
                // Handle the 'Up' case
                Last_Row = Row;
                Last_Column = Column;
                if (Row - 1 < 0)
                {
                    Row = Total_Rows - 1;
                }
                else
                {
                    Row = Row - 1;
                }
                MenuMoved = true;
                break;

            case EMenuType::VE_Down:
                // Handle the 'Down' case
                Last_Row = Row;
                Last_Column = Column;
                if (Row + 1 >= Total_Rows)
                {
                    Row = 0;
                }
                else
                {
                    Row = Row + 1;
                }
                MenuMoved = true;
                break;

            case EMenuType::VE_Right:
                // Handle the 'Right' case
                Last_Row = Row;
                Last_Column = Column;
                if (Column + 1 >= Total_Columns)
                {
                    Column = 0;
                }
                else
                {
                    Column = Column + 1;
                }
                Row_Init = Row;
                Row = 0;
                MenuMoved = true;
                break;

            case EMenuType::VE_Left:
                // Handle the 'Left' case
                Last_Row = Row;
                Last_Column = Column;
                if (Column - 1 < 0)
                {
                    Column = Total_Columns - 1;
                }
                else
                {
                    Column = Column - 1;
                }
                Row = Row_Init;
                MenuMoved = true;
                break;

            case EMenuType::VE_Confirm:
                // Handle the 'Confirm' case
                MenuMoved = true;
                break;

            case EMenuType::VE_Back:
                // Handle the 'Back' case
                MenuMoved = true;
                break;

            default:
                // Handle unexpected cases
                break;
        }
    } else {

        switch (MenuPress)
        {
            case EMenuType::VE_None:
                // Handle the 'None' case
                break;

            case EMenuType::VE_Up:
                // Handle the 'Up' case
                Last_Row = Row;
                Last_Column = Column;
                if (Row - 1 < 0)
                {
                    Row = Total_Rows - 1;
                }
                else
                {
                    Row = Row - 1;
                }
                MenuMoved = true;
                break;

            case EMenuType::VE_Down:
                // Handle the 'Down' case
                Last_Row = Row;
                Last_Column = Column;
                if (Row + 1 >= Total_Rows)
                {
                    Row = 0;
                }
                else
                {
                    Row = Row + 1;
                }
                MenuMoved = true;
                break;

            case EMenuType::VE_Right:
                // Handle the 'Right' case
                Last_Row = Row;
                Last_Column = Column;
                if (Column + 1 >= Total_Columns)
                {
                    Column = 0;
                    if(Row + 1 >= Total_Rows){
                        Row = 0;
                    } else {
                        Row = Row + 1;
                    }
                    
                }
                else
                {
                    Column = Column + 1;
                }
               
                MenuMoved = true;
                break;

            case EMenuType::VE_Left:
                // Handle the 'Left' case
                Last_Row = Row;
                Last_Column = Column;
                if (Column - 1 < 0)
                {
                    Column = Total_Columns - 1;
                    if (Row - 1 < 0)
                    {
                        Row = Total_Rows - 1;
                    }
                    else
                    {
                        Row = Row - 1;
                    }

                }
                else
                {
                    Column = Column - 1;
                }
                
                MenuMoved = true;
                break;

            case EMenuType::VE_Confirm:
                // Handle the 'Confirm' case
                MenuMoved = true;
                break;

            case EMenuType::VE_Back:
                // Handle the 'Back' case
                MenuMoved = true;
                break;

            default:
                // Handle unexpected cases
                break;
        }

    }
}




void ABasePlayerController::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);

        //Replicate current health.
        DOREPLIFETIME(ABasePlayerController, IsNetwork_Player);
        DOREPLIFETIME(ABasePlayerController, Simulator_Character_Set);
        DOREPLIFETIME(ABasePlayerController, PlayerNumber);

        
}