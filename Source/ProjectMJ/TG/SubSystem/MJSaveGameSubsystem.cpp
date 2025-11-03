// Fill out your copyright notice in the Description page of Project Settings.


#include "MJSaveGameSubsystem.h"

#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TG/MJGameInstance.h"
#include "TG/Struct/MJSaveGame.h"

UMJSaveGameSubsystem::UMJSaveGameSubsystem()
{
	
	static ConstructorHelpers::FClassFinder<UUserWidget> LoadingScreenRef(TEXT("/Game/TG/TestWBP/WBP_LoadingScreen.WBP_LoadingScreen_C"));
	if (LoadingScreenRef.Succeeded())
	{
		LoadingScreen = LoadingScreenRef.Class;
	}
	MaxSaveSlotNum = 8;
}

void UMJSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UMJSaveGameSubsystem::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMJSaveGameSubsystem::EndLoadingScreen);
}

void UMJSaveGameSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	//SaveGameToCurrentSlotNum();
	
}

void UMJSaveGameSubsystem::BeginLoadingScreen(const FString& MapName)
{
	
	FLoadingScreenAttributes LoadingScreenAttr;

	UUserWidget* Buffer = CreateWidget<UUserWidget>(GetGameInstance(), LoadingScreen);
		
	LoadingScreenAttr.WidgetLoadingScreen = Buffer->TakeWidget();
	LoadingScreenAttr.bAutoCompleteWhenLoadingCompletes = false;
	LoadingScreenAttr.MinimumLoadingScreenDisplayTime = 2.0f;
 
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttr);
}

void UMJSaveGameSubsystem::EndLoadingScreen(UWorld* InLoadedWorld)
{
	
}

UMJSaveGame* UMJSaveGameSubsystem::GetSaveGameData()
{
	return SaveGameData;		
}

void UMJSaveGameSubsystem::SaveGameToCurrentSlotNum()
{

	UMJGameInstance* MJGI = Cast<UMJGameInstance>(GetGameInstance());
	if (MJGI)
	{
		const uint8 SlotNum = MJGI->GetPlayerSessionDataRef().SaveGameSlotNum;

		if (MJGI->GetPlayerSessionDataRef().SaveGameSlotNum == INDEX_NONE)
		{
			return;
		}
		
		const FString SlotName = FString::Printf(TEXT("Slot_%d"), SlotNum);

		UMJSaveGame* SaveGame = Cast<UMJSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

		if (!SaveGame)
		{
			SaveGame = Cast<UMJSaveGame>(UGameplayStatics::CreateSaveGameObject(UMJSaveGame::StaticClass()));
			SaveGame->SlotNum = SlotNum;
		}

		SaveGame->PlayerName = MJGI->GetPlayerSessionDataRef().PlayerName;
		SaveGame->PlayerLevel = MJGI->GetPlayerSessionDataRef().PlayerLevel;
		SaveGame->PlayerExp = MJGI->GetPlayerSessionDataRef().PlayerExp;
		SaveGame->SlotNum = MJGI->GetPlayerSessionDataRef().SaveGameSlotNum;
		SaveGame->RecentPlayedDateTime = FDateTime::Now();

		SaveGame->SetCurrentEquippedSkillMap(MJGI->GetPlayerSessionDataRef().CurrentEquippedSkillMap);
		SaveGame->SetCurrentOwnedSKillMap(MJGI->GetPlayerSessionDataRef().CurrentOwnedSkillMap);

		
		UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	}
	
	
}

void UMJSaveGameSubsystem::SaveGameToSelectedSlotNum(const uint8 InputSlotNum)
{
	// using only in SavePointActor
	UMJGameInstance* MJGI = Cast<UMJGameInstance>(GetGameInstance());
	if (MJGI)
	{
		const FString SlotName = FString::Printf(TEXT("Slot_%d"), InputSlotNum);

		UMJSaveGame* SaveGame;

		if (UGameplayStatics::DoesSaveGameExist(SlotName,0))
		{
			SaveGame = Cast<UMJSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		}
		else
		{
			SaveGame = Cast<UMJSaveGame>(UGameplayStatics::CreateSaveGameObject(UMJSaveGame::StaticClass()));
		}

		if (SaveGame)
		{
			SaveGame->PlayerName = MJGI->GetPlayerSessionDataRef().PlayerName;
			SaveGame->PlayerLevel = MJGI->GetPlayerSessionDataRef().PlayerLevel;
			SaveGame->PlayerExp = MJGI->GetPlayerSessionDataRef().PlayerExp;
			SaveGame->SlotNum = InputSlotNum;
			SaveGame->RecentPlayedDateTime = FDateTime::Now();
			SaveGame->SaveGameCreatedDateTime = FDateTime::Now();
			
			SaveGame->SetCurrentEquippedSkillMap(MJGI->GetPlayerSessionDataRef().CurrentEquippedSkillMap);
			SaveGame->SetCurrentOwnedSKillMap(MJGI->GetPlayerSessionDataRef().CurrentOwnedSkillMap);
		}

		UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	}
}

bool UMJSaveGameSubsystem::LoadGameFromSlotNum(int8 SlotNum)
{
	
	UMJGameInstance* MJGI = Cast<UMJGameInstance>(GetGameInstance());
	if (MJGI)
	{
		const FString SlotName = FString::Printf(TEXT("Slot_%d"), SlotNum);
		if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			UMJSaveGame* SaveGame = Cast<UMJSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

			MJGI->GetPlayerSessionDataRef().PlayerName = SaveGame->PlayerName;
			MJGI->GetPlayerSessionDataRef().PlayerLevel = SaveGame->PlayerLevel;
			MJGI->GetPlayerSessionDataRef().PlayerExp = SaveGame->PlayerExp;
			MJGI->GetPlayerSessionDataRef().SaveGameSlotNum = SaveGame->SlotNum;
			MJGI->GetPlayerSessionDataRef().SetCurrentEquippedSkillMap(SaveGame->CurrentEquippedSkillMap);
			MJGI->GetPlayerSessionDataRef().SetCurrentOwnedSKillMap(SaveGame->CurrentOwnedSkillMap);
			
			return true;
		}
	}
	return false;
}

uint8 UMJSaveGameSubsystem::GetCurrentSavedSlotNum() const
{
	uint8 Result = 0;
	
	for (int i = 0; i < MaxSaveSlotNum; ++i)
	{
		const FString SlotName = FString::Printf(TEXT("Slot_%d"), i);
    		
		if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			++Result;	
		}
	}

	return Result;
}

bool UMJSaveGameSubsystem::IsSlotFull() const
{
	if (GetCurrentSavedSlotNum() == MaxSaveSlotNum)
	{
		return true;
	}
	return false;
}
