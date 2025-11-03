// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Attributes/MJCharacterAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "TG/Struct/MJPlayerSessionDataStruct.h"
#include "MJPlayerState.generated.h"


/**
 * Class Description: Player의 ASC 정보를 담는 곳, AttributeSet도 여기서 가진다
 * Author: 이주현
 * Created Date: 미상
 * Last Modified By: 신동민
 * Last Modified Date: 2025-05-20
 */
UCLASS()
class PROJECTMJ_API AMJPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMJPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UMJCharacterAttributeSet* GetCharacterAttributeSet() const;

	FMJPlayerSessionData& GetPlayerSessionDataRef();

	UFUNCTION()
	void SaveToInstancedPlayerSessionData();

	UFUNCTION()
	void LoadFromInstancedPlayerSessionData();

	
protected:

	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "GAS")
	TObjectPtr<class UMJAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<class UMJCharacterAttributeSet> CharacterAttributeSet;

	UPROPERTY()
	TObjectPtr<class UMJCharacterSkillAttributeSet> CharacterSkillAttributeSet;

	UPROPERTY()
	FMJPlayerSessionData PlayerSessionData;
	
};



