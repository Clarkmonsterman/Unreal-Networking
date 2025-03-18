// Copyright Epic Games, Inc. All Rights Reserved.

#include "PoliticalUndergroundGameMode.h"
#include "BasePlayerController.h"
#include "Net/UnrealNetwork.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "PoliticalGameStateBase.h"
#include "UObject/ConstructorHelpers.h"



// Copyright Epic Games, Inc. All Rights Reserved.



APoliticalUndergroundGameMode::APoliticalUndergroundGameMode()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	PlayerControllerClass = ABasePlayerController::StaticClass();
	numRounds = 3;
	defaultRoundTime = 90.0f;
	roundTime = 90.0f;
	isTimerActive = false;
	PlayerNum = 0;
	bUseSeamlessTravel = true;
    bReplicates = true;
    bNetLoadOnClient = true;
}

void APoliticalUndergroundGameMode::PostLogin(APlayerController* NewPlayer){
	Super::PostLogin(NewPlayer);

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Post-Login Called")));
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Join In Progress %s"), JoinInProgess ? TEXT("True") : TEXT("False")));

	bool localControl = NewPlayer->IsLocalController();
	
	Connected_Controllers.Add(NewPlayer);

	APoliticalGameStateBase* GS = GetWorld()->GetGameState<APoliticalGameStateBase>();
	if(GS){
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Local Control? %s"), localControl ? TEXT("True") : TEXT("False")));;
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Players: %d"), Num_Players));

		if(!localControl){
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Should Spawn Pawns ")));
			SpawnNetworkPawns();
			
		}
	}
	
	//SpawnNetworkPawns();
}


void APoliticalUndergroundGameMode::GenericPlayerInitialization(AController* Controller){
	Super::GenericPlayerInitialization(Controller);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Generic Player Init Called")));
}


void APoliticalUndergroundGameMode::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);

        //Replicate current health.
        //DOREPLIFETIME(APoliticalUndergroundGameMode, JoinInProgess);
}




void APoliticalUndergroundGameMode::BeginPlay(){
	IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().AddUObject(this, &APoliticalUndergroundGameMode::OnControllerChanged);

}


void APoliticalUndergroundGameMode::OnControllerChanged(EInputDeviceConnectionState connectionState, FPlatformUserId userID, FInputDeviceId inputDeviceID){
	UE_LOG(LogTemp, Warning, TEXT("Controller changed!"));

	if (connectionState == EInputDeviceConnectionState::Disconnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("Controller disconnected!"));
		int32 userID_int = userID.GetInternalId();
		int32 inputDeviceID_int = inputDeviceID.GetId();
		UE_LOG(LogTemp, Display, TEXT("FPlatFormUserID: %d"), userID_int);
		UE_LOG(LogTemp, Display, TEXT("FInputDeviceID: %d"), inputDeviceID_int);
		
	}

	if (connectionState == EInputDeviceConnectionState::Connected)
	{
		UE_LOG(LogTemp, Warning, TEXT("Controller disconnected!"));
		int32 userID_int = userID.GetInternalId();
		int32 inputDeviceID_int = inputDeviceID.GetId();
		UE_LOG(LogTemp, Display, TEXT("FPlatFormUserID: %d"), userID_int);
		UE_LOG(LogTemp, Display, TEXT("FInputDeviceID: %d"), inputDeviceID_int);
		
	}



}


void APoliticalUndergroundGameMode::AddBaseController(ABasePlayerController* controller){
	Game_Controllers.Add(controller);
	
}


void APoliticalUndergroundGameMode::CheckControllerRef(ABasePlayerController* controller){
	if(controller == Game_Controllers[1]){
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Controller Matches #2")));
	}
}