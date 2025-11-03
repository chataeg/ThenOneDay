// ThenOneDayStudio

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MJDungeonMapWidget.generated.h"

class UBackgroundBlur;
class UCanvasPanel;
class UImage;
/**
 * Class Description: DungeonMapWidget
 * Author: Cha Tae Gwan
 * Created Date: 2025_07_29
 * Last Modified By: Cha Tae Gwan
 * Last Modified Date: 2025_07_29
 */


UCLASS()
class PROJECTMJ_API UMJDungeonMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UFUNCTION()
	void SetAllVisibility(ESlateVisibility NewVisibility);
	
protected:
	
	virtual void NativeConstruct() override;
	
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UFUNCTION()
	void DrawNode();

	UFUNCTION()
	void DrawEdge();

	UFUNCTION()
	void DrawAllWidget();
	
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NodeWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> EdgeWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
	
	UPROPERTY()
	TArray<FVector2D> PivotPoints;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBackgroundBlur> BackgroundBlur;
	
};
