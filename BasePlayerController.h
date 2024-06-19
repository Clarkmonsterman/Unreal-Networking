// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h" 
#include "PoliticalUndergroundCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PoliticalGameStateBase.h"
#include "DonaldRenderMachine.h"
#include "BasePlayerController.generated.h"



/**
 * 
 */


UENUM(BlueprintType)
enum class EInputType : uint8
{
	VE_None UMETA(DisplayName = "None"),
	VE_Forward UMETA(DisplayName = "Forward"),
	VE_Back UMETA(DisplayName = "Back"),
	VE_Up UMETA(DisplayName = "Up"),
	VE_Down UMETA(DisplayName = "Down"),
	VE_Attack1 UMETA(DisplayName = "Attack1"),
	VE_Attack2 UMETA(DisplayName = "Attack2"),
	VE_Attack3 UMETA(DisplayName = "Attack3"),
	VE_Attack4 UMETA(DisplayName = "Attack4"),
	VE_SPAttack1 UMETA(DisplayName = "SPAttack1"),
	VE_SPAttack2 UMETA(DisplayName = "SPAttack2"),
	VE_Hold UMETA(DisplayName = "Hold"),
	VE_Hold_Release UMETA(DisplayName = "Hold_Release"),
	VE_Block UMETA(DisplayName = "Block"),
	VE_Block_Release UMETA(DisplayName = "Block_Release"),
	VE_Jump UMETA(DisplayName = "Jump"),
	VE_Buffer UMETA(DisplayName = "Buffer"),
	VE_Release UMETA(DisplayName = "Release"),
	VE_Sync UMETA(DisplayName = "Sync"),
	
	// must go back through .cpp file when sobererrr.... Lol

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
class ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABasePlayerController();

	/* Use BeginPlay to start the functionality */
	virtual void BeginPlay() override;

	//void PossessCharacter(FString CharacterToPosess)
	UFUNCTION(BlueprintCallable)
	void PossessCharacter(APawn* CharacterToPosess);

	virtual void SetupInputComponent() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	EMenuType MenuPress;

	int32 controllerTicks;

	int32 postSpawnTicks;


	UFUNCTION(BlueprintCallable)
	void CallMoveRight(float _value);

	UFUNCTION(BlueprintCallable)
	void CallMoveForward(float _value);

	UFUNCTION(BlueprintCallable)
	void CallJump();
	

	bool nullController;

	APoliticalGameStateBase* GameState;

	bool GameStateCast;

	void NetworkGameStateCast_Attempt();

	UFUNCTION(BlueprintCallable)
	void CallStartBlocking();
	UFUNCTION(BlueprintCallable)
	void CallStopBlocking();
	UFUNCTION(BlueprintCallable)
	void CallAttack1();
	UFUNCTION(BlueprintCallable)
	void CallAttack2();
	UFUNCTION(BlueprintCallable)
	void CallAttack3();
	UFUNCTION(BlueprintCallable)
	void CallAttack4();
	UFUNCTION(BlueprintCallable)
	void CallSpecialAttack1();
	UFUNCTION(BlueprintCallable)
	void CallSpecialAttack2();
	UFUNCTION(BlueprintCallable)
	void CallHoldPressed();
	UFUNCTION(BlueprintCallable)
	void CallHoldReleased();

	UFUNCTION(BlueprintCallable)
	void AddToBuffer();

	UFUNCTION(BlueprintCallable)
	void ReleaseFromBuffer();

	
	UFUNCTION(BlueprintCallable)
	void CallMenuRight();
	UFUNCTION(BlueprintCallable)
	void CallMenuLeft();
	UFUNCTION(BlueprintCallable)
	void CallMenuUp();
	UFUNCTION(BlueprintCallable)
	void CallMenuDown();
	UFUNCTION(BlueprintCallable)
	void CallMenuConfirm();
	UFUNCTION(BlueprintCallable)
	void CallMenuBack();

	UFUNCTION(BlueprintCallable)
	void ResetMenuSelection();

	int64 MenuSelectFrame;
	int64 ActionSelectFrame;


	void DetermineInputDeviceDetails(FKey _keyPressed);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
	bool isInputDeviceGamepad;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
	int32 LocalPlayer_Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
	int32 PlatFormUser_ID;



	UPROPERTY(Replicated)
	bool Simulator_Character_Set;


	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void Update_Status(bool status, int PlayerNum);
	virtual void Update_Status_Implementation(bool status, int PlayerNum);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void SpawnCharactersLocal();
	virtual void SpawnCharactersLocal_Implementation();


	void SpawnCharacter(FVector SpawnLoc, FRotator SpawnRot, int _player);

	void  SpawnGameCamera();

	TStrongObjectPtr<APoliticalUndergroundCharacter> Player_One;
	TStrongObjectPtr<APoliticalUndergroundCharacter> Player_Two;


	APoliticalUndergroundCharacter* Weak_PlayerOne;
	APoliticalUndergroundCharacter* Weak_PlayerTwo;
	bool CharacterCastToGameState;


	UPROPERTY(Replicated)
	bool IsNetwork_Player;

	UPROPERTY(Replicated)
	int PlayerNumber;


	void SendLocalInputs();


	

	void GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const;

private:
	UPROPERTY(Replicated)
	EInputType inputPress;

	
};
