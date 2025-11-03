// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TG/Interface/MJBossEventManager.h"
#include "TG/DataTable/MJWaveAISpawnRow.h"
#include "TG/Struct/MJDungeonSessionDataStruct.h"
#include "MJGameStateDungeon.generated.h"

/**
 * Class Description: 던전의 상태를 저장할 GameState
 * Author: 차태관
 * Created Date: 2025-06-13
 * Last Modified By: 차태관
 * Last Modified Date: 2025-06-13
 */

class UMJMiniMapIconMeshComponent;
class AMJForestCreatureCharacter;
class UEnvQuery;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMJAIBossOnHealthChangedSignature, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMJAIBossOnSpawnedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMJAIBossOnDiedSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMJActorSpawnedSignature, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMJAIOnDestroyedSignature);

UCLASS()
class PROJECTMJ_API AMJGameStateDungeon : public AGameStateBase, public IMJBossEventManager
{
	GENERATED_BODY()
	
public:
	// Initialize Section

	AMJGameStateDungeon();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	
	UFUNCTION(BlueprintCallable)
	void SetDungeonSessionData(const FMJDungeonSessionData& DungeonSessionData);
	
	UFUNCTION(BlueprintCallable)
	void SaveToInstancedDungeonSessionData(uint8 SaveToNum);

	UFUNCTION()
	void LoadFromInstancedDungeonSessionData(uint8 LoadFromNum);

	// Miscellaneous
	
	UPROPERTY(BlueprintAssignable)
	FMJAIBossOnHealthChangedSignature OnAIBossHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FMJActorSpawnedSignature OnActorSpawned;

	UPROPERTY(BlueprintAssignable)
	FMJAIBossOnSpawnedSignature OnAIBossSpawned;

	UPROPERTY(BlueprintAssignable)
	FMJAIBossOnDiedSignature OnAIBossDied;


	UFUNCTION(BlueprintCallable)
	virtual void PublishOnBossSpawned() override;
	
protected:

	
	// Initialize Section
	UFUNCTION()
	void Initialize_BattleNode();

	UFUNCTION()
	void Initialize_BossNode();

	UFUNCTION()
	void Initialize_RewardNode();
	
	// AI Spawn Section
	
	UFUNCTION(BlueprintCallable)
	bool GetWaveDataRowByIndex(int32 InputWaveRowNum);
	
	UFUNCTION(BlueprintCallable)
	void CheckSpawnAICondition();
	
	UFUNCTION(BlueprintCallable)
	void SpawnAI();
	
	UFUNCTION(BlueprintCallable)
	TSubclassOf<AActor> GetActorFromPool();

	UFUNCTION(BlueprintCallable)
	void Test_DeleteAI();
	
	UFUNCTION(BlueprintCallable)
	void OnAIDestroy(AActor* DestroyedActor);

	UFUNCTION()
	void SpawnDungeonPortal();
	
	// Dungeon Session Section
	
	UPROPERTY(BlueprintReadOnly)
	FMJDungeonSessionData LoadedDungeonSessionData;

	UPROPERTY(BlueprintReadOnly)
	uint8 LoadedDungeonNodeNum;

	// Static AISpawn Section
	
	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> StaticSpawnPointActors;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> LoadedStaticDataTable;
	
	UPROPERTY(EditDefaultsOnly)
	int32 StaticAISpawnMaxNum;
	
	// Wave Section
	
	UPROPERTY()
	FTimerHandle WaveAISpawn_ConditionCheckTimerHandle;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UEnvQuery> EQSQuery_WaveRandomSpawn;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> LoadedWaveDataTable;

	UPROPERTY(BlueprintReadOnly)
	FMJWaveAISpawnDataRow LoadedWaveDataRow;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentWaveNum;

	// AI Spawn Condition Section

	UPROPERTY(BlueprintReadOnly)
	int32 WaveAISpawnMaxNum;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentSpawnedAINum;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SpawnedActorRefs;
	
	UPROPERTY(BlueprintAssignable)
	FMJAIOnDestroyedSignature OnAIDestroyed;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> DungeonPortalActorClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> EndPortalActorClass;

	UPROPERTY()
	TSubclassOf<AActor> TargetPortalToSpawn;
	
	UPROPERTY()
	FTimerHandle DungeonPortalSpawnTimerHandle;

	// Boss Section

	UPROPERTY()
	TObjectPtr<AMJForestCreatureCharacter> BossAIRef;

	UPROPERTY()
	FTimerHandle OnBossSpawnedBroadCastTimerHandle;
	
};
