// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/MJGA_GameplayAbility.h"
#include "MJGA_InstantSkill.generated.h"

/**
 * Class Description: �÷��̾��� ��ų�� Instant, Charging, Passive�� ����
 * Instant�� ��Ŭ�� ���￡ ��Ī�ϴ� ��ų + �⺻ ����
 * Author: �ŵ���
 * Created Date: 2025_06_24
 * Last Modified By: (Last Modifier)
 * Last Modified Date: (Last Modified Date)
 */

class UAnimMontage;
class UGameplayEffect;

UCLASS()
class PROJECTMJ_API UMJGA_InstantSkill : public UMJGA_GameplayAbility
{
	GENERATED_BODY()

public:
	UMJGA_InstantSkill();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> SkillActionAnimMontage;
};
