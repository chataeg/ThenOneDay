// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TG/Struct/MJDungeonGraphStruct.h"
#include "MJDungeonGenerationSubSystem.generated.h"

/**
 * Class Description: GameInstance Subsystem generates DungeonGraph
 * Author: Cha Tae Gwan
 * Created Date: 2025-06-20
 * Last Modified By: Cha Tae Gwan
 * Last Modified Date: 2025-06-20
 */
UCLASS()
class PROJECTMJ_API UMJDungeonGenerationSubSystem : public UGameInstanceSubsystem
{
   GENERATED_BODY()

public:
   
   UMJDungeonGenerationSubSystem();
   
   UFUNCTION(BlueprintCallable)
   void GenerateDungeonGraph();
   
   UFUNCTION(BlueprintCallable)
   bool CheckHasRoute(uint8 CurrentNodeNum, uint8 DestNodeNum);
   
   FMJDungeonGraph* GetDungeonGraph()  { return &DungeonGraph; }

   UFUNCTION(BlueprintCallable, Category="Dungeon")
   uint8 GetMaxNodeNum() const;
   
   UFUNCTION(BlueprintCallable, Category="Dungeon")
   void SetMaxNodeNum(uint8 NewMaxNodeNum);

protected:
   
   // GeneratingDungeonSystem Section

   UPROPERTY(BlueprintReadOnly)
   FMJDungeonGraph DungeonGraph;
   
   UPROPERTY(BlueprintReadOnly)
   uint8 MaxNodeNum;

   UFUNCTION()
   FMJDungeonNode MakeNewNode(uint8 NodeNum, uint8 AssignedMapID,  EMJNodeType NodeType, EMJAISpawnType AISpawnType, FVector2D UICoord);
   
   UFUNCTION()
   void ConnectNodesByDistance(float MaxDistance, int MaxEdgePerNode);

   UFUNCTION()
   void ConnectNodesByMST(float MaxDistance);

   UFUNCTION()
   bool CheckHasIterableGraph(); 

   UFUNCTION()
   FVector2D GetQuadBezier(float t, const FVector2D StartPoint, const FVector2D EndPoint, const FVector2D ControlPoint);
   
   
};
