// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <atomic>
#include <string>
#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h" 
#include "../PoliticalUndergroundCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PoliticalGameStateBase.h"
#include "BasePlayerController.generated.h"



/**
 * 
 */

 UENUM(BlueprintType)
enum class EPoliticalCharacter : uint8
{
	VE_None UMETA(DisplayName = "None"),
	VE_Boden UMETA(DisplayName = "Boden"),
	VE_Tremp UMETA(DisplayName = "Tremp"),
	

};


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
	VE_ReleaseAttack1 UMETA(DisplayName = "ReleaseAttack1"),
	VE_ReleaseAttack2 UMETA(DisplayName = "ReleaseAttack2"),
	VE_ReleaseAttack3 UMETA(DisplayName = "ReleaseAttack3"),
	VE_ReleaseAttack4 UMETA(DisplayName = "ReleaseAttack4"),
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
	void ReleaseAttack1();
	UFUNCTION(BlueprintCallable)
	void CallAttack2();
	UFUNCTION(BlueprintCallable)
	void ReleaseAttack2();
	UFUNCTION(BlueprintCallable)
	void CallAttack3();
	UFUNCTION(BlueprintCallable)
	void ReleaseAttack3();
	UFUNCTION(BlueprintCallable)
	void CallAttack4();
	UFUNCTION(BlueprintCallable)
	void ReleaseAttack4();
	UFUNCTION(BlueprintCallable)
	void CallSpecialAttack1();
	UFUNCTION(BlueprintCallable)
	void CallSpecialAttack2();
	UFUNCTION(BlueprintCallable)
	void CallHoldPressed();
	UFUNCTION(BlueprintCallable)
	void CallHoldReleased();

	
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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	bool Continous;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	bool MenuMoved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	EMenuType MenuPress;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	int Row = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	int Row_Init = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	int Column = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	int Total_Rows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	int Total_Columns;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	int Last_Row;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	int Last_Column;

	void MoveSelection();

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
	void SetMenuInputs();


	

	void GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const;

private:
	
	int32 inputPressed;
	void convertInputsPressed(int32 input);
	

};
