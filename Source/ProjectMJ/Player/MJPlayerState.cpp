// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MJPlayerState.h"
#include "ProjectMJ.h"
#include "AbilitySystem/MJAbilitySystemComponent.h"
#include "AbilitySystem//Attributes/MJCharacterAttributeSet.h"
#include "AbilitySystem/Attributes/MJCharacterSkillAttributeSet.h"
#include "Character/MJPlayerCharacter.h"
#include "Character/Component/MJPlayerSkillComponent.h"
#include "Character/Component/MJPlayerStatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TG/MJGameInstance.h"

AMJPlayerState::AMJPlayerState()
{
	ASC = CreateDefaultSubobject<UMJAbilitySystemComponent>(TEXT("ASC"));
	CharacterAttributeSet = CreateDefaultSubobject<UMJCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	CharacterSkillAttributeSet = CreateDefaultSubobject<UMJCharacterSkillAttributeSet>(TEXT("CharacterSkillAttributeSet"));
}

UAbilitySystemComponent* AMJPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

UMJCharacterAttributeSet* AMJPlayerState::GetCharacterAttributeSet() const
{
	return CharacterAttributeSet;
}

FMJPlayerSessionData& AMJPlayerState::GetPlayerSessionDataRef()
{
	return PlayerSessionData;
}

void AMJPlayerState::SaveToInstancedPlayerSessionData()
{
	FMJPlayerSessionData& MJGIPlayerSessionData = GetGameInstance<UMJGameInstance>()->GetPlayerSessionDataRef();
	
	UMJPlayerStatComponent* PlayerStatComp = GetPawn()->FindComponentByClass<UMJPlayerStatComponent>();
	if (PlayerStatComp)
	{
		MJGIPlayerSessionData.PlayerLevel = PlayerStatComp->GetPlayerLevel();
		MJGIPlayerSessionData.PlayerExp = PlayerStatComp->GetTotalCumulativeExperience();
	}

	UMJPlayerSkillComponent* PlayerSkillComponent = GetPawn()->FindComponentByClass<UMJPlayerSkillComponent>();
	if (PlayerSkillComponent)
	{

		MJGIPlayerSessionData.SetCurrentEquippedSkillMap(PlayerSkillComponent->GetEquippedSkillMap());
		MJGIPlayerSessionData.SetCurrentOwnedSKillMap(PlayerSkillComponent->GetOwnedSkillMap());
	}
}

void AMJPlayerState::LoadFromInstancedPlayerSessionData()
{
	PlayerSessionData = GetGameInstance<UMJGameInstance>()->GetPlayerSessionDataRef();
	PlayerSessionData.PlayerLevel =  GetGameInstance<UMJGameInstance>()->GetPlayerSessionDataRef().PlayerLevel;
	PlayerSessionData.PlayerExp = GetGameInstance<UMJGameInstance>()->GetPlayerSessionDataRef().PlayerExp;

	if (AMJPlayerCharacter* MJPlayer = Cast<AMJPlayerCharacter>(GetPawn()))
	{
		if (UMJPlayerStatComponent* PlayerStatComp = MJPlayer->FindComponentByClass<UMJPlayerStatComponent>())
		{
			PlayerStatComp->SetPlayerLevel(PlayerSessionData.PlayerLevel);
			PlayerStatComp->SetTotalCumulativeExperience(PlayerSessionData.PlayerExp);
			PlayerStatComp->InitializeStat();
		}
		UMJPlayerSkillComponent* PlayerSkillComponent = GetPawn()->FindComponentByClass<UMJPlayerSkillComponent>();
		if (PlayerSkillComponent)
		{
			PlayerSkillComponent->SetOwnedSkillMap(PlayerSessionData.CurrentOwnedSkillMap);
			PlayerSkillComponent->SetEquippedSkillMap(PlayerSessionData.CurrentEquippedSkillMap);
		}
	
	}
}

void AMJPlayerState::BeginPlay()
{
	Super::BeginPlay();

	LoadFromInstancedPlayerSessionData();
}

