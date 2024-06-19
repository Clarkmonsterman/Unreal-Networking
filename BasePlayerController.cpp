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
        //UE_LOG(LogTemp, Warning, TEXT("LocalPlayer ID: %d"), LocalPlayer_Id);
       // UE_LOG(LogTemp, Display, TEXT("PlatFormID: %d"), PlatFormUser_ID);
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
    } else {
        LocalPlayer_Id = -10;
        //UE_LOG(LogTemp, Warning, TEXT("Controller ID: %d"), Id);
    }
    
    
    if(MenuSelectFrame > 0){
        --MenuSelectFrame;
    } else if(MenuSelectFrame == 0){
        //UE_LOG(LogTemp, Display, TEXT("Reset Menu Select"));
        MenuPress = EMenuType::VE_None;
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
            if(postSpawnTicks == 60){
                Player_Two.Get()->StartFight();
                Player_One.Get()->StartFight();
            }
            if(HasAuthority() && postSpawnTicks == 70){
                inputPress = EInputType::VE_Sync;
                ActionSelectFrame = 1;
            }
            
        }
    }
    

    controllerTicks++;

    if(GameStateCast && IsNetwork_Player){
        
        if(ActionSelectFrame > 0){
            SendLocalInputs();
            ActionSelectFrame = 0;
        } else if(ActionSelectFrame == 0){
            inputPress = EInputType::VE_None;
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
	InputComponent->BindAction("Attack2", IE_Pressed, this, &ABasePlayerController::CallAttack2);
	InputComponent->BindAction("Attack3", IE_Pressed, this, &ABasePlayerController::CallAttack3);
	InputComponent->BindAction("Attack4", IE_Pressed, this, &ABasePlayerController::CallAttack4);
	InputComponent->BindAction("SpecialAttack1", IE_Pressed, this, &ABasePlayerController::CallSpecialAttack1);
	InputComponent->BindAction("SpecialAttack2", IE_Pressed, this, &ABasePlayerController::CallSpecialAttack2);
	InputComponent->BindAction("Hold", IE_Pressed, this, &ABasePlayerController::CallHoldPressed);
	InputComponent->BindAction("Hold", IE_Released, this, &ABasePlayerController::CallHoldReleased);
    InputComponent->BindAction("AddToInputBuffer", IE_Pressed, this, &ABasePlayerController::AddToBuffer);
    InputComponent->BindAction("AddToInputBuffer", IE_Released, this, &ABasePlayerController::ReleaseFromBuffer);
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

    if(IsNetwork_Player){
        APawn* ControlledPawn = GetPawn();
        if(!ControlledPawn){
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("No Possessed Pawn")));
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("PlayerNumber: %d"), PlayerNumber));
        } else {
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Possessed Pawn")));
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("PlayerNumber: %d"), PlayerNumber));
        }
    }
    
    
}


void ABasePlayerController::AddToBuffer(){
    //inputPress = EInputType::VE_Buffer;
    //ActionSelectFrame = 1;
}
void ABasePlayerController::ReleaseFromBuffer(){
    //inputPress = EInputType::VE_Release;
    //ActionSelectFrame = 1;
}

void ABasePlayerController::CallMoveRight(float _value){

    if(_value > 0){
        inputPress = EInputType::VE_Forward;
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Move Forward")));
        ActionSelectFrame = 1;
    } else if(_value < 0){
        inputPress = EInputType::VE_Back;
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Move Backward")));
        ActionSelectFrame = 1;
    }

    
}


void ABasePlayerController::CallMoveForward(float _value){
     
    
    
    if(_value > 0){
        inputPress = EInputType::VE_Up;
        ActionSelectFrame = 1;
    } else if(_value < 0) {
        inputPress = EInputType::VE_Down;
        ActionSelectFrame = 1;
    }

    
}

void ABasePlayerController::CallJump(){

   inputPress = EInputType::VE_Jump;
   ActionSelectFrame = 1;
}

void ABasePlayerController::CallStartBlocking(){

    inputPress = EInputType::VE_Block;
    ActionSelectFrame = 1;
}

void ABasePlayerController::CallStopBlocking(){

    inputPress = EInputType::VE_Block_Release;
    ActionSelectFrame = 1;
}

void ABasePlayerController::CallAttack1(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Attack1")));
    inputPress = EInputType::VE_Attack1;
    ActionSelectFrame = 1;
}

void ABasePlayerController::CallAttack2(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Attack2")));
    inputPress = EInputType::VE_Attack2;
    ActionSelectFrame = 1;
}

void ABasePlayerController::CallAttack3(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Attack3")));
    inputPress = EInputType::VE_Attack3;
    ActionSelectFrame = 1;
}

void ABasePlayerController::CallAttack4(){
    //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Call Attack4")));
   
    inputPress = EInputType::VE_Attack4;
    ActionSelectFrame = 1;
    
}

void ABasePlayerController::CallSpecialAttack1(){

    inputPress = EInputType::VE_SPAttack1;
    ActionSelectFrame = 1;
}
void ABasePlayerController::CallSpecialAttack2(){

    inputPress = EInputType::VE_SPAttack2;
    ActionSelectFrame = 1;
}

void ABasePlayerController::CallHoldPressed(){

    inputPress = EInputType::VE_Hold;
    ActionSelectFrame = 1;
}
void ABasePlayerController::CallHoldReleased(){

    inputPress = EInputType::VE_Hold_Release;
    ActionSelectFrame = 1;
}



void ABasePlayerController::SendLocalInputs(){
   uint8 retVal = static_cast<uint8>(inputPress);
   GameState->RetrieveLocalInputs_Implementation(retVal);
}


void ABasePlayerController::NetworkGameStateCast_Attempt(){
    
    GameState = Cast<APoliticalGameStateBase>(GetWorld()->GetGameState());
    if(GameState){
        GameState->SetClientCharacter_Implementation(Weak_PlayerOne);
        GameState->SetConnectedCharacter_Implementation(Weak_PlayerTwo); 
        GameState->GetConnectedPlayerNetworkInfo_Implementation();
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
                
        UObject* SpawnActor = Cast<UObject>(StaticLoadObject(UObject::StaticClass(), NULL, TEXT("/Game/ThirdPerson/Blueprints/BP_Don.BP_Don")));
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
                FRotator SpawnRot= PlayerStart->GetActorRotation();


                
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
        //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Characters Valid")));
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



void ABasePlayerController::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);

        //Replicate current health.
        DOREPLIFETIME(ABasePlayerController, IsNetwork_Player);
        DOREPLIFETIME(ABasePlayerController, Simulator_Character_Set);
        DOREPLIFETIME(ABasePlayerController, PlayerNumber);
        DOREPLIFETIME(ABasePlayerController,inputPress);

        
}