// ThenOneDayStudio


#include "TG/UI/MJDungeonMapWidget.h"

#include "MJDungeonNodeWidget.h"
#include "Components/BackgroundBlur.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetMathLibrary.h"
#include "TG/SubSystem/MJDungeonGenerationSubSystem.h"


void UMJDungeonMapWidget::NativeConstruct()
{	
	Super::NativeConstruct();

	//Background->SetVisibility(ESlateVisibility::Hidden);
	BackgroundBlur->SetVisibility(ESlateVisibility::Hidden);
	DrawAllWidget();
}

int32 UMJDungeonMapWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// Draw Debug Lines
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void UMJDungeonMapWidget::DrawNode()
{
	const FMJDungeonGraph* DungeonGraph = GetGameInstance()->GetSubsystem<UMJDungeonGenerationSubSystem>()->GetDungeonGraph();
	
	if (DungeonGraph)
	{
		for (auto& Iter : DungeonGraph->Nodes)
		{
			UMJDungeonNodeWidget* NewNodeWidget = Cast<UMJDungeonNodeWidget>(CreateWidget(this, NodeWidgetClass));
			if (NewNodeWidget)
			{
				NewNodeWidget->NodeNum->SetText(FText::FromString(FString::FromInt(Iter.NodeID)));
				NewNodeWidget->SetVisibility(ESlateVisibility::Hidden);

				switch (Iter.NodeType)
				{
					case EMJNodeType::Battle:

						break;
				case EMJNodeType::Boss:
						NewNodeWidget->SetColorAndOpacity(FLinearColor::Red);
					
						break;
				case EMJNodeType::Reward:
						NewNodeWidget->SetColorAndOpacity(FLinearColor::Green);
						break;
				}
				
				UCanvasPanelSlot* AssignedSlot = Cast<UCanvasPanelSlot>(CanvasPanel->AddChild(NewNodeWidget));
				
				if (AssignedSlot)
				{
					AssignedSlot->SetPosition(FVector2d(Iter.UICoord.X,Iter.UICoord.Y));
					
				}
				
			}
		}
	}
	
	
}

void UMJDungeonMapWidget::DrawEdge()
{
	const FMJDungeonGraph* DungeonGraph = GetGameInstance()->GetSubsystem<UMJDungeonGenerationSubSystem>()->GetDungeonGraph();

	if (DungeonGraph)
	{
		for (auto& IterNode : DungeonGraph->Nodes)
		{
			for (auto& IterNodeConnectedNodeID : IterNode.ConnectedNodeIDs)
			{
				FVector2D NewPivotPoint = (IterNode.UICoord + DungeonGraph->Nodes[IterNodeConnectedNodeID].UICoord) / 2 + 50.f;

				
				if (!PivotPoints.Contains(NewPivotPoint))
				{
					PivotPoints.AddUnique(NewPivotPoint);

					UUserWidget* NewEdgeWidget = CreateWidget(this,EdgeWidgetClass);
					if (NewEdgeWidget)
					{
						FVector2D DeltaVector = DungeonGraph->Nodes[IterNodeConnectedNodeID].UICoord - IterNode.UICoord;

						float Distance2D = UKismetMathLibrary::Distance2D(DungeonGraph->Nodes[IterNodeConnectedNodeID].UICoord, IterNode.UICoord);

						float Angle = FMath::RadiansToDegrees(FMath::Atan2(DeltaVector.Y, DeltaVector.X));
						
						NewEdgeWidget->SetRenderTransform(FWidgetTransform(NewPivotPoint,FVector2D(1.0f,1.0f),FVector2D::ZeroVector, Angle));
						NewEdgeWidget->SetVisibility(ESlateVisibility::Hidden);
						
						UCanvasPanelSlot* AssignedSlot = CanvasPanel->AddChildToCanvas(NewEdgeWidget);
						AssignedSlot->SetAlignment(FVector2D(0.5f,0.5f));
						AssignedSlot->SetSize(FVector2D(Distance2D,10.0f));
						
					}
					
				}
			}
		}
	}
}

void UMJDungeonMapWidget::DrawAllWidget()
{
	PivotPoints.Empty();
	DrawEdge();
	DrawNode();
}


void UMJDungeonMapWidget::SetAllVisibility(ESlateVisibility NewVisibility)
{
	for (auto& Iter : CanvasPanel->GetAllChildren())
	{
		Iter->SetVisibility(NewVisibility);
	}
}
