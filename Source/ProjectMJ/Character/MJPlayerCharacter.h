// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/MJCharacterBase.h"
#include "UI/Inventory/MJInventoryComponent.h"
#include "UI/Inventory/MJInventoryInterface.h"
#include "MJPlayerCharacter.generated.h"

class UMJPlayerEffectComponent;
class UMJPlayerStatComponent;
class UMJPlayerSkillComponent;
class USpringArmComponent;
class UCameraComponent;
class UMJPlayerCombatComponent;
class UAIPerceptionStimuliSourceComponent;
class USphereComponent;
class UMJInventoryComponent;
class UMJFadeObjectComponent;

/**
 * Class Description:
 * Author: Lee JuHyeon
 * Created Date: 2025_06_11
 * Modified By: 신동민
 * Modified Date: 2025.07.08
 * Modified Description: Add SkillComponent
 * Modified By: 김민진
 * Modified Date: 2025.07.29.
 * Modified Description: Add ID
 * Modified By: Lee JUHYEON
 * Modified Date: 2025.08.04
 * Modified Description: Add Weapon
 */
UCLASS()
class PROJECTMJ_API AMJPlayerCharacter : public AMJCharacterBase, public IMJInventoryInterface
{
	GENERATED_BODY()
	
public:
	AMJPlayerCharacter();
	
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController)override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// AI Perception
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAIPerceptionStimuliSourceComponent> PerceptionStimuliSourceComponent;
	
private:

#pragma region ComponentPart
public:
	FORCEINLINE UMJPlayerCombatComponent* GetPlayerCombatComponent() { return PlayerCombatComponent; }
	

#pragma region DialoguePart	
protected:
 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly ,Category = "Trigger")
 	TObjectPtr<USphereComponent> UITrigger;

 	UPROPERTY()
 	TObjectPtr<AActor> UITarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UMJInventoryComponent> InventoryComponent;

	UPROPERTY()
	TSubclassOf<AActor>WeaponClass;
public:
	void SetUITarget(AActor* NewTarget) { UITarget = NewTarget; }
	
	AActor* GetUITarget() {return UITarget;}
	USphereComponent* GetUITrigger() {return UITrigger;}
	UMJInventoryComponent* GetInventoryComponent() {return InventoryComponent;}


	//태관  25.07.23 add protected:
protected:
	
#pragma endregion
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category="Camera",meta=(AllowPrivateAccess=true))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMJPlayerCombatComponent>PlayerCombatComponent;

	//FadeActor System Part
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Fade", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMJFadeObjectComponent>FadeComponent;
#pragma endregion

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMJPlayerSkillComponent> SkillComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMJPlayerStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMJPlayerEffectComponent> EffectComponent;

	UFUNCTION()
	void OnDamage(float Magnitude, bool bIsCritical);

};
