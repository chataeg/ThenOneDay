// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MJSaveGameSubsystem.generated.h"

/**
 * Class Description: GameInstance Subsystem saves GameData
 * Author: Cha Tae Gwan
 * Created Date: 2025-06-20
 * Last Modified By: Cha Tae Gwan
 * Last Modified Date: 2025-06-20
 */

class UMJSaveGame;

UCLASS()
class PROJECTMJ_API UMJSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UMJSaveGameSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	UMJSaveGame* GetSaveGameData();

	UFUNCTION()
	void SaveGameToCurrentSlotNum();

	UFUNCTION()
	void SaveGameToSelectedSlotNum(const uint8 InputSlotNum);

	UFUNCTION()
	bool LoadGameFromSlotNum(int8 SlotNum);

	UFUNCTION()
	uint8 GetCurrentSavedSlotNum() const;
	
	FORCEINLINE uint8 GetMaxSaveSlotNum() const {return MaxSaveSlotNum; }

	bool IsSlotFull() const;

	// Loading Screen Section	
	
	UFUNCTION()
	virtual void BeginLoadingScreen(const FString& MapName);
	UFUNCTION()
	virtual void EndLoadingScreen(UWorld* InLoadedWorld);

	
protected:
	
	UPROPERTY(VisibleAnywhere)
	TSubclassOf<UUserWidget> LoadingScreen;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMJSaveGame> SaveGameData;

	UPROPERTY(VisibleAnywhere)
	uint8 MaxSaveSlotNum;
	
};
