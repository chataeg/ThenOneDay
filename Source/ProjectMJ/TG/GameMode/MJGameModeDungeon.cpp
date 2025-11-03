// Fill out your copyright notice in the Description page of Project Settings.


#include "MJGameModeDungeon.h"

#include "Kismet/GameplayStatics.h"
#include "Player/MJPlayerState.h"
#include "TG/MJGameInstance.h"
#include "TG/GameState/MJGameStateDungeon.h"


AMJGameModeDungeon::AMJGameModeDungeon()
{
}

void AMJGameModeDungeon::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AMJGameModeDungeon::TravelToMapByNode(const FString MapName, const uint8 NodeNum)
{

	UMJGameInstance* MJGI = GetGameInstance<UMJGameInstance>();
	if (MJGI)
	{
		AMJPlayerState* PS = Cast<AMJPlayerState>(UGameplayStatics::GetPlayerState(this,0));
		
		MJGI->GetPlayerSessionDataRef().CurrentDungeonMapNum = NodeNum;
		
		AMJGameStateDungeon* MJGS = GetGameState<AMJGameStateDungeon>();
		
		if (MJGS )
		{
			MJGS->SaveToInstancedDungeonSessionData(PS->GetPlayerSessionDataRef().CurrentDungeonMapNum);
		}
		
		MJGI->GetPlayerSessionDataRef().CurrentDungeonMapNum = NodeNum;
		
	}
	
	return Super::TravelToMapByNode(MapName, NodeNum);
}

