// Fill out your copyright notice in the Description page of Project Settings.


#include "MJGameStateDungeon.h"

#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "Character/MJForestCreatureCharacter.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Kismet/GameplayStatics.h"
#include "TG/MJGameInstance.h"
#include "TG/Actor/MJDungeonAISpawnPointActor.h"
#include "TG/DataTable/MJStaticAISpawnRow.h"
#include "TG/Interface/MJInstancedActorInterface.h"
#include "TG/SubSystem/MJDungeonGenerationSubSystem.h"


AMJGameStateDungeon::AMJGameStateDungeon()
{
	LoadedDungeonNodeNum = 255;

	CurrentWaveNum = 1;
	WaveAISpawnMaxNum = 5;

	StaticAISpawnMaxNum = 10;

	CurrentSpawnedAINum = 0;
}

void AMJGameStateDungeon::BeginPlay()
{
	Super::BeginPlay();

	UMJDungeonGenerationSubSystem* GS = GetGameInstance()->GetSubsystem<UMJDungeonGenerationSubSystem>();
	
	UMJGameInstance * MJGI = GetGameInstance<UMJGameInstance>();
	if (MJGI)
	{
		LoadFromInstancedDungeonSessionData(MJGI->GetPlayerSessionDataRef().CurrentDungeonMapNum);
	}
		
	if (GS)
	{
		EMJNodeType CurrentNodeType = GS->GetDungeonGraph()->Nodes[LoadedDungeonSessionData.DungeonNodeNum].NodeType;
		
		switch (CurrentNodeType)
		{
			case EMJNodeType::Battle:
				Initialize_BattleNode();		
				break;
			
			case EMJNodeType::Boss:
				Initialize_BossNode();
				break;
			
			case EMJNodeType::Reward:
				Initialize_RewardNode();
				break;
			default:
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, FString::Printf(TEXT("InValid!!!")));
				break;
		}
	}
	else
	{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, FString::Printf(TEXT("GameState is nullptr!!!")));
	}
}

void AMJGameStateDungeon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (auto& Iter : SpawnedActorRefs)
	{
		if (Iter.IsValid())
		{
			Iter.Get()->OnDestroyed.RemoveDynamic(this,&AMJGameStateDungeon::OnAIDestroy);

			AMJCharacterBase* Char = Cast<AMJCharacterBase>(Iter.Get());
			if (IsValid(Char))
			{
				Char->GetMesh()->GetAnimInstance()->StopAllMontages(0.2f);
			}
			
		}
	}
	GetWorldTimerManager().ClearAllTimersForObject(this);
	

}


void AMJGameStateDungeon::Initialize_BattleNode()
{
	if (LoadedDungeonSessionData.DungeonContext == EMJDungeonContext::InActive)
	{
		if (LoadedDungeonSessionData.AISpawnType == EMJAISpawnType::Static)
		{
			UGameplayStatics::GetAllActorsOfClass(GetWorld(),AMJDungeonAISpawnPointActor::StaticClass(),StaticSpawnPointActors);

			// using StaticAISpawnData 
			TArray<const FMJStaticAISpawnDataRow*> LoadedAllRows;
			LoadedStaticDataTable->GetAllRows(TEXT("Load All StaticAISpawnData Rows"),LoadedAllRows);

			float TotalWeight = 0.0f;

			for (const auto& Iter  : LoadedAllRows)
			{
				TotalWeight += Iter->SpawnWeight;
			}
			
			
			for (const auto& Iter : StaticSpawnPointActors)
			{
				AMJDungeonAISpawnPointActor* SpawnPointActor = Cast<AMJDungeonAISpawnPointActor>(Iter);

				if (IsValid(SpawnPointActor))
				{
					FVector IterSpawnPoint = Iter->GetActorLocation();
					uint8 IterSpawnAIMaxNum = SpawnPointActor->NumberToSpawn;
						
					FNavLocation ResultLocation;
	
					UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
				
					if (NavSys)
					{
						int32 CurrentPointSpawnedAINum = 0;
						while (CurrentPointSpawnedAINum < IterSpawnAIMaxNum)
						{
							TSubclassOf<AActor> WeightedChosenClass;

							float RandWeight = FMath::RandRange(0.0f,TotalWeight);

							float AccumulatedWeight = 0.0f;
						
							for (const auto& Row : LoadedAllRows)
							{
								AccumulatedWeight += Row->SpawnWeight;

								if (AccumulatedWeight >= RandWeight)
								{
									WeightedChosenClass = Row->EnemyClass;
									break;
								}
							}
						
							bool bIsFound = NavSys->GetRandomPointInNavigableRadius(IterSpawnPoint, 1000.f,ResultLocation);

							if (bIsFound)
							{
								FActorSpawnParameters SpawnParams;
								SpawnParams.Owner = this;
								SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
						
							
								AActor* NewAIActor = GetWorld()->SpawnActor<AActor>(WeightedChosenClass,ResultLocation,FRotator(), SpawnParams);
							
								check(NewAIActor);

								if (NewAIActor)
								{
									NewAIActor->OnDestroyed.AddDynamic(this, &AMJGameStateDungeon::OnAIDestroy);
									SpawnedActorRefs.Add(NewAIActor);
									++CurrentSpawnedAINum;
									++CurrentPointSpawnedAINum;

									OnActorSpawned.Broadcast(NewAIActor);
								}
								else
								{
									GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("AISpawn Failed!!")));
								}
							}
						}
					}
				}
			}
		}
		else if (LoadedDungeonSessionData.AISpawnType == EMJAISpawnType::Wave)
		{
			GetWorldTimerManager().SetTimer(WaveAISpawn_ConditionCheckTimerHandle, this, &AMJGameStateDungeon::CheckSpawnAICondition,  2.0f, true);

			if (LoadedWaveDataTable)
			{
				GetWaveDataRowByIndex(CurrentWaveNum);
			}
		}

		LoadedDungeonSessionData.DungeonContext = EMJDungeonContext::Activated;
		
	}
		
}

void AMJGameStateDungeon::Initialize_BossNode()
{
	if (LoadedDungeonSessionData.DungeonContext == EMJDungeonContext::InActive)
	{
		BossAIRef = Cast<AMJForestCreatureCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(),AMJForestCreatureCharacter::StaticClass()));
		if (BossAIRef)
		{
			BossAIRef->OnDestroyed.AddDynamic(this ,&AMJGameStateDungeon::OnAIDestroy);
			SpawnedActorRefs.Add(BossAIRef);
			CurrentSpawnedAINum++;
			GetWorldTimerManager().SetTimer(OnBossSpawnedBroadCastTimerHandle,this, &ThisClass::PublishOnBossSpawned,3.0f);

			OnActorSpawned.Broadcast(BossAIRef);
			OnAIBossSpawned.Broadcast();
		}
		LoadedDungeonSessionData.DungeonContext = EMJDungeonContext::Activated;
	}
}

void AMJGameStateDungeon::Initialize_RewardNode()
{
	
}

bool AMJGameStateDungeon::GetWaveDataRowByIndex(int32 InputWaveRowNum)
{
	// Hard coded wave name for now
	FName Name = *FString::Printf(TEXT("Wave%d"), InputWaveRowNum);
	FMJWaveAISpawnDataRow* RowPtr = LoadedWaveDataTable->FindRow<FMJWaveAISpawnDataRow>(Name, TEXT("FindRow is Failed"));
	
	if (RowPtr)
	{
		LoadedWaveDataRow.EnemyCount = RowPtr->EnemyCount;
		LoadedWaveDataRow.EnemyPool = RowPtr->EnemyPool;
		LoadedWaveDataRow.WaveNum = RowPtr->WaveNum;

		return true;
	}
	return false;
}



void AMJGameStateDungeon::CheckSpawnAICondition()
{
	if (LoadedDungeonSessionData.DungeonContext != EMJDungeonContext::Activated)
	{
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Magenta, FString::Printf(TEXT("Check AISpawn Condition Call")));
	
	if (CurrentSpawnedAINum < WaveAISpawnMaxNum)
	{
		if (LoadedWaveDataRow.EnemyCount <= 0)
		{
			bool GetNextWave = GetWaveDataRowByIndex(CurrentWaveNum + 1);

			if (GetNextWave)
			{
				CurrentWaveNum += 1;
			}
			else if (!GetNextWave && CurrentSpawnedAINum <= 0)
			{
					GetWorldTimerManager().ClearTimer(WaveAISpawn_ConditionCheckTimerHandle);
					LoadedDungeonSessionData.DungeonContext = EMJDungeonContext::Cleared;
					TargetPortalToSpawn = DungeonPortalActorClass;
					GetWorldTimerManager().SetTimer(DungeonPortalSpawnTimerHandle, this, &AMJGameStateDungeon::SpawnDungeonPortal, 2.0f, false);
			}
		}
		else
		{
			SpawnAI();
		}
	}
}

void AMJGameStateDungeon::SpawnAI()
{
	if (LoadedDungeonSessionData.DungeonContext != EMJDungeonContext::Activated)
	{
		return;
	}
	
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this,0);

	if (Player)
	{
		FEnvQueryRequest Request(EQSQuery_WaveRandomSpawn, Player);

		// Async Call Lamda func
		Request.Execute(EEnvQueryRunMode::AllMatching, FQueryFinishedSignature::CreateLambda([this, Player](TSharedPtr<FEnvQueryResult> Result)
				   {
					   if (Result->IsSuccessful())
					   {
							   TArray<FVector> AllLocations;

							   AllLocations.Reserve(Result->Items.Num());
	
							   Result->GetAllAsLocations(AllLocations);
								
							   int i = 0;
							   while (CurrentSpawnedAINum < WaveAISpawnMaxNum)
							   {
							   	
							   		FActorSpawnParameters SpawnParams;
							   	  SpawnParams.Owner = this;
							   		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

							   		TSubclassOf<AActor> GetActor = GetActorFromPool();
								    AActor* NewAIActor = GetWorld()->SpawnActor<AActor>(GetActor, AllLocations[i],FRotator(), SpawnParams);

							   		//check(NewAIActor);
							   	
							   	   if (NewAIActor)
								   {
										
							   		   // Add Delegate when it`s spawned by GameState
									   NewAIActor->OnDestroyed.AddDynamic(this, &AMJGameStateDungeon::OnAIDestroy);
									   SpawnedActorRefs.Add(NewAIActor);
									   ++CurrentSpawnedAINum;
									   --LoadedWaveDataRow.EnemyCount;
									   ++i;

							   	   	OnActorSpawned.Broadcast(NewAIActor);
							   	   	
								   }
								   else
								   {
									   GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("AISpawn Failed!!")));
								   }
							   }
					   }
				   }));
	}
}

void AMJGameStateDungeon::OnAIDestroy(AActor* DestroyedActor)
{
	if (DestroyedActor->IsPendingKillPending())
	{
		--CurrentSpawnedAINum;

		UMJDungeonGenerationSubSystem* GS = GetGameInstance()->GetSubsystem<UMJDungeonGenerationSubSystem>();

		if (GS)
		{
			EMJNodeType CurrentNodeType = GS->GetDungeonGraph()->Nodes[LoadedDungeonSessionData.DungeonNodeNum].NodeType;

			EMJAISpawnType CurrentAISpawnType = LoadedDungeonSessionData.AISpawnType;
			bool bIsActivatedMap = (LoadedDungeonSessionData.DungeonContext == EMJDungeonContext::Activated);
			bool bIsAllEnemyDied = CurrentSpawnedAINum <= 0;
			
			if (CurrentNodeType == EMJNodeType::Battle)
			{
				if (CurrentAISpawnType == EMJAISpawnType::Static && bIsActivatedMap && bIsAllEnemyDied)
				{
					LoadedDungeonSessionData.DungeonContext = EMJDungeonContext::Cleared;
					TargetPortalToSpawn = DungeonPortalActorClass;
			
					GetWorldTimerManager().SetTimer(DungeonPortalSpawnTimerHandle, this, &AMJGameStateDungeon::SpawnDungeonPortal, 2.0f, false);
				}
				else if (CurrentAISpawnType == EMJAISpawnType::Wave && bIsActivatedMap && bIsAllEnemyDied )
				{
					// Wave AISpawn Mode`s End called at CheckAISpawn callback
				}
				
			}
			else if (CurrentNodeType == EMJNodeType::Boss)
			{
				if (CurrentAISpawnType == EMJAISpawnType::Static && bIsActivatedMap && bIsAllEnemyDied)
				{
					LoadedDungeonSessionData.DungeonContext = EMJDungeonContext::Cleared;
					TargetPortalToSpawn = EndPortalActorClass;

					OnAIBossDied.Broadcast();
					GetWorldTimerManager().SetTimer(DungeonPortalSpawnTimerHandle, this, &AMJGameStateDungeon::SpawnDungeonPortal, 2.0f, false);
				}
			}
			else if (CurrentNodeType == EMJNodeType::Reward)
			{
				
			}
		}
	}
}

TSubclassOf<AActor> AMJGameStateDungeon::GetActorFromPool()
{
	// pick Actor class randomly
	TArray<TSubclassOf<AActor>> Keys;
	LoadedWaveDataRow.EnemyPool.GetKeys(Keys);

	if (Keys.Num() == 0)
	{
		return nullptr;
	}
	
	const int32 RandomIndex = FMath::RandRange(0,Keys.Num() - 1);

	if (Keys[RandomIndex] == nullptr)
	{
		return GetActorFromPool();
	}
	
	return Keys[RandomIndex];
}

void AMJGameStateDungeon::Test_DeleteAI()
{
	for (auto& Iter : SpawnedActorRefs)
	{
		if (Iter.IsValid())
		{
			Iter.Get()->Destroy();
		}
	}
}

void AMJGameStateDungeon::SpawnDungeonPortal()
{
	if (LoadedDungeonSessionData.DungeonContext != EMJDungeonContext::Cleared)
	{
		return;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this,0);
	if (Player)
	{
		FEnvQueryRequest Request(EQSQuery_WaveRandomSpawn, Player);

		Request.Execute(EEnvQueryRunMode::AllMatching, FQueryFinishedSignature::CreateLambda([this, Player](TSharedPtr<FEnvQueryResult> Result)
		{
			if (Result->IsSuccessful())
			{
				FVector Location;
				
				Location = Result->GetItemAsLocation(0);

				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				
				AActor* Portal = GetWorld()->SpawnActor<AActor>(TargetPortalToSpawn,Location,FRotator(),Params);
				if (Portal)
				{
					GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, FString::Printf(TEXT("PortalActor is Successfully Spawned")));

					OnActorSpawned.Broadcast(Portal);
				}
			}
		}));
		
	}
}


void AMJGameStateDungeon::SetDungeonSessionData(const FMJDungeonSessionData& DungeonSessionData)
{
	LoadedDungeonSessionData = DungeonSessionData;
}

void AMJGameStateDungeon::SaveToInstancedDungeonSessionData(uint8 SaveToNum)
{
	UMJGameInstance* MJGI = GetGameInstance<UMJGameInstance>();

	FMJDungeonSessionData NewDungeonSessionData = LoadedDungeonSessionData;
	NewDungeonSessionData.SpawnInfos.Empty();
	
	if (MJGI) 
	{
		for (TActorIterator<AActor> Iter(GetWorld()); Iter ; ++Iter)
		{
			if (Iter->Implements<UMJInstancedActorInterface>())
			{
				AActor* Actor = *Iter;
				FMJDungeonActorInfo Info;
				Info.ActorClass = Actor->GetClass();
				Info.Transform = Actor->GetActorTransform();
				NewDungeonSessionData.SpawnInfos.Add(Info);	
			}
		}
		
		MJGI->GetDungeonSessionDataRef()[SaveToNum] = NewDungeonSessionData;
	} 
}

void AMJGameStateDungeon::LoadFromInstancedDungeonSessionData(uint8 LoadFromNum)
{
	UMJGameInstance* MJGI = GetGameInstance<UMJGameInstance>();

	if (MJGI)
	{
		SetDungeonSessionData(MJGI->GetDungeonSessionDataRef()[LoadFromNum]);
 
		for (auto iter : LoadedDungeonSessionData.SpawnInfos)
		{
			if (iter.ActorClass->ImplementsInterface(UMJInstancedActorInterface::StaticClass()))
			{
				
				UClass* LoadedClass = iter.ActorClass.LoadSynchronous();
				if (LoadedClass)
				{
					FActorSpawnParameters Params;
					Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
						
					AActor* SavedActor = GetWorld()->SpawnActorDeferred<AActor>(LoadedClass,iter.Transform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
					if (SavedActor)
					{
						SavedActor->FinishSpawning(iter.Transform);
						OnActorSpawned.Broadcast(SavedActor);
					}
				}
			}
		}
	}
}


void AMJGameStateDungeon::PublishOnBossSpawned()
{
	OnAIBossSpawned.Broadcast();
}