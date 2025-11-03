// ThenOneDayStudio


#include "TG/UI/MJBossHpBarWidget.h"

#include "AbilitySystem/MJAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MJCharacterAttributeSet.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"

void UMJBossHpBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMJBossHpBarWidget::BindToAttributes()
{
	// AActor* BossRef = UGameplayStatics::GetActorOfClass(GetWorld(),AMJAIBossCharacterTG::StaticClass());
	//
	// if (BossRef)
	// {
	// 	AMJAIBossCharacterTG* MJBossRef = Cast<AMJAIBossCharacterTG>(BossRef);
	//
	// 	if (MJBossRef)
	// 	{
	// 		UMJAbilitySystemComponent* MJBossASC = Cast<UMJAbilitySystemComponent>(MJBossRef->GetAbilitySystemComponent());
	// 		
	// 		const UMJCharacterAttributeSet* MJBossASet = MJBossASC->GetSet<UMJCharacterAttributeSet>();
	// 		if (MJBossASC)
	// 		{
	// 			if (MJBossASC)
	// 			{
	// 				
	// 				CurrentHealth = MJBossASC->GetNumericAttribute(UMJCharacterAttributeSet::GetHealthAttribute());
	// 				CurrentMaxHealth = MJBossASC->GetNumericAttribute(UMJCharacterAttributeSet::GetMaxHealthAttribute());
	// 				
	// 				MJBossASC->GetGameplayAttributeValueChangeDelegate(MJBossASet->GetHealthAttribute()).AddUObject(this,&UMJBossHpBarWidget::OnHealthChanged);		
	// 				MJBossASC->GetGameplayAttributeValueChangeDelegate(MJBossASet->GetMaxHealthAttribute()).AddUObject(this,&UMJBossHpBarWidget::OnMaxHealthChanged);
	// 				
	// 			}
	// 		}	
	// 	}
	// }
}

void UMJBossHpBarWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue >= 0.0f)
	{
		CurrentHealth = FMath::Clamp(Data.NewValue, 0.0f, CurrentMaxHealth);
		HpProgressBar->SetPercent(CurrentHealth / CurrentMaxHealth);

		if (FMath::IsNearlyZero(CurrentHealth))
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UMJBossHpBarWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue >= 0.0f)
	{
		CurrentMaxHealth = Data.NewValue;
		HpProgressBar->SetPercent(CurrentHealth/ CurrentMaxHealth);
	}
}

