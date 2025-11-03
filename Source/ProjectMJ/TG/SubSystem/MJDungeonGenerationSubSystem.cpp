// Fill out your copyright notice in the Description page of Project Settings.


#include "MJDungeonGenerationSubSystem.h"
#include <functional>
#include "TG/MJGameInstance.h"


UMJDungeonGenerationSubSystem::UMJDungeonGenerationSubSystem()
{
	MaxNodeNum = 10;
}

void UMJDungeonGenerationSubSystem::GenerateDungeonGraph()
{
	UMJGameInstance* MJGI = Cast<UMJGameInstance>(GetGameInstance());
	
	check(MJGI);

	// Initialize Data
	DungeonGraph = FMJDungeonGraph();
	MJGI->GetDungeonSessionDataRef().Empty();
	
	FVector2D StartPoint;
	FVector2D EndPoint;

	FVector2D ControlPoint;
	uint8 CurrentNotAssignedNodeNum = 0;
	
	FVector2D ViewportSize = FVector2D(1920.f,1080.f);

	// Sets Bezier Curve Points (for now, just one curve)
	StartPoint = FVector2D(ViewportSize.X / 4,ViewportSize.Y * 0.5);
	EndPoint = FVector2D(ViewportSize.X - ViewportSize.X / 4,ViewportSize.Y * 0.5);
	ControlPoint = FVector2D(ViewportSize.X/2,ViewportSize.Y * 0.05);

	while (CurrentNotAssignedNodeNum < MaxNodeNum)
	{
		// divide curve and make nodes
		float t = static_cast<float>(CurrentNotAssignedNodeNum) / (MaxNodeNum - 1);
		
		FVector2D BezierPoint = GetQuadBezier(t, StartPoint, EndPoint, ControlPoint);
		DungeonGraph.BezierPoints.Add(BezierPoint);
	
		float OffsetPivot = 200.f;
		
		FVector2D RandomOffset = FVector2D(FMath::RandRange(-OffsetPivot,OffsetPivot), FMath::RandRange(-OffsetPivot,OffsetPivot));
		FVector2D BasePosition = BezierPoint;
		FVector2D CandidatePoint = BasePosition + RandomOffset;
		
		// When Node is outside of Viewpoert, re-generates node.
		if (CandidatePoint.X < 0.f || CandidatePoint.X > ViewportSize.X ||
			CandidatePoint.Y < 0.f || CandidatePoint.Y > ViewportSize.Y)
		{
			continue;
		}
		
		// When Created Node Position is too close, re-generates node.
		uint8 TooCloseNodesNum = 0;
		for (auto& iter : DungeonGraph.Nodes)
		{
			if (FVector2D::Distance(CandidatePoint, iter.UICoord) < OffsetPivot )
			{
				TooCloseNodesNum++;
			}
		}
		
		if (TooCloseNodesNum > 0)
		{
			continue;
		}

		// Determine Map`s Type
		float Prob = FMath::RandRange(0.0f,100.f);
		
		// First, Check is there any boss room in DungeonGraph
		if (CurrentNotAssignedNodeNum == MaxNodeNum - 1 && DungeonGraph.BossNodeID == -1)
		{
			// Boss
			
			FMJDungeonNode NewNode = MakeNewNode(CurrentNotAssignedNodeNum,0,EMJNodeType::Boss, EMJAISpawnType::Static, CandidatePoint);
			DungeonGraph.Nodes.Add(NewNode);
			DungeonGraph.BossNodeID = CurrentNotAssignedNodeNum;

			MJGI->GetDungeonSessionDataRef().Add(FMJDungeonSessionData(EMJDungeonContext::InActive, CurrentNotAssignedNodeNum, EMJAISpawnType::Static, FString(TEXT("Boss"))));
	
			CurrentNotAssignedNodeNum++;
			
		}
		else if (Prob > 0.0f && Prob < 80.f)
		{	
			// Battle 1~5 Map Assign
			// AISpawn Wave Map : 01 02
			// AISpawn Static Map : 03 04 05 
			float RandNum = FMath::RandRange(1,5);

			EMJAISpawnType AISpawnType = EMJAISpawnType::Static;
			
			// Wave 
			if (RandNum == 1 || RandNum == 2)
			{
				AISpawnType = EMJAISpawnType::Wave;
				
			}
			
			FMJDungeonNode NewNode = MakeNewNode(CurrentNotAssignedNodeNum,RandNum,EMJNodeType::Battle, AISpawnType, CandidatePoint);
			DungeonGraph.Nodes.Add(NewNode);

			MJGI->GetDungeonSessionDataRef().Add(FMJDungeonSessionData(EMJDungeonContext::InActive, CurrentNotAssignedNodeNum, AISpawnType, FString(TEXT("Battle"))));
			
			CurrentNotAssignedNodeNum++;
		}
		else if (Prob >= 80.f && Prob < 90.f)
		{
			// Reward
			FMJDungeonNode NewNode = MakeNewNode(CurrentNotAssignedNodeNum,0,EMJNodeType::Reward, EMJAISpawnType::Static, CandidatePoint);
			DungeonGraph.Nodes.Add(NewNode);

			
			MJGI->GetDungeonSessionDataRef().Add(FMJDungeonSessionData(EMJDungeonContext::InActive, CurrentNotAssignedNodeNum, EMJAISpawnType::Static, FString(TEXT("Reward"))));
	
			CurrentNotAssignedNodeNum++;
		}
		else if (Prob >= 90.f && Prob < 100.f)
		{
			// Boss
			bool HasBossRoom = false;
			
			for (auto iter : DungeonGraph.Nodes)
			{
				if (iter.NodeType == EMJNodeType::Boss)
				{
					HasBossRoom = true;
					break;
				}
			}
			
			if (HasBossRoom)
			{
				continue;
			}
			
			FMJDungeonNode NewNode = MakeNewNode(CurrentNotAssignedNodeNum,0,EMJNodeType::Boss, EMJAISpawnType::Static, CandidatePoint);
			DungeonGraph.Nodes.Add(NewNode);
			DungeonGraph.BossNodeID = CurrentNotAssignedNodeNum;

			MJGI->GetDungeonSessionDataRef().Add(FMJDungeonSessionData(EMJDungeonContext::InActive, CurrentNotAssignedNodeNum, EMJAISpawnType::Static,  FString(TEXT("Boss"))));
	
			CurrentNotAssignedNodeNum++;
		}
	}

	
	

	// Choose most far node from BossNode
	float MaxDist = FLT_MIN;
	uint8 MaxDistNodeNum = -1;
	for (auto& iter : DungeonGraph.Nodes)
	{
		if (iter.NodeType  != EMJNodeType::Battle)
			continue;
		
		if (MaxDist < FVector2D::Distance(DungeonGraph.Nodes[iter.NodeID].UICoord, DungeonGraph.Nodes[DungeonGraph.BossNodeID].UICoord))
		{
			MaxDist = FVector2D::Distance(DungeonGraph.Nodes[iter.NodeID].UICoord, DungeonGraph.Nodes[DungeonGraph.BossNodeID].UICoord);
			MaxDistNodeNum = iter.NodeID;
			
		}
	}
	
	DungeonGraph.StartNodeID = MaxDistNodeNum;

	if (MJGI)
	{
		
		MJGI->GetPlayerSessionDataRef().CurrentDungeonMapNum = DungeonGraph.StartNodeID;
		
	}
	
	ConnectNodesByMST(FVector2D::Distance(FVector2D(0.0f,0.0f), ViewportSize));
	
	ConnectNodesByDistance(400.f,4);

	if (!CheckHasIterableGraph())
	{
		return GenerateDungeonGraph();
	}
}


FMJDungeonNode UMJDungeonGenerationSubSystem::MakeNewNode(uint8 NodeNum, uint8 AssignedMapID, EMJNodeType NodeType,EMJAISpawnType AISpawnType, FVector2D UICoord)
{
	FMJDungeonNode NewNode;

	NewNode.NodeID = NodeNum;
	NewNode.AssignedMapID = AssignedMapID;
	NewNode.NodeType = NodeType;
	NewNode.AISpawnType = AISpawnType;
	NewNode.UICoord = UICoord;
	
	return NewNode;
}

void UMJDungeonGenerationSubSystem::ConnectNodesByDistance(float MaxDistance, int MaxEdgePerNode)
{
	const int32 NodeCount = DungeonGraph.Nodes.Num();

	for (int i = 0 ; i < NodeCount ; i++)
	{
		FMJDungeonNode& CurrentNode = DungeonGraph.Nodes[i];

		TArray<TPair<uint32, float>> CandidateNodes;

		for (int j = 0 ; j < NodeCount ; j++)
		{
			if (i == j) continue;

			FMJDungeonNode& IterNode = DungeonGraph.Nodes[j];
			
			float DistanceBetweenNodes = FVector2D::Distance(CurrentNode.UICoord, IterNode.UICoord);
			
			if (DistanceBetweenNodes <= MaxDistance)
			{
				CandidateNodes.Add(TPair<uint32, float>(j,DistanceBetweenNodes));
			}
		}

		CandidateNodes.Sort([](const TPair<uint32, float> A, const TPair<uint32,float> B)
		{
			return A.Value < B.Value;
		});

		for (int k = 0 ; k < CandidateNodes.Num() ; k++)
		{
			int32 TargetIndex = CandidateNodes[k].Key;

			// 양방향 연결 체크 후 추가
			if (!CurrentNode.ConnectedNodeIDs.Contains(TargetIndex) &&
				!DungeonGraph.Nodes[TargetIndex].ConnectedNodeIDs.Contains(i) &&
				CurrentNode.ConnectedNodeIDs.Num() < MaxEdgePerNode)
				//&& DungeonGraph.Nodes[TargetIndex].ConnectedNodeIDs.Num() < FMath::RandRange(2,MaxEdgePerNode))
			{
				CurrentNode.ConnectedNodeIDs.Add(TargetIndex);
				DungeonGraph.Nodes[TargetIndex].ConnectedNodeIDs.Add(i);
			}
		}
	}
	
}

void UMJDungeonGenerationSubSystem::ConnectNodesByMST(float MaxDistance)
{
	const uint8 NodeCount = DungeonGraph.Nodes.Num();

	if ( NodeCount <= 1 ) return;
	
	TArray<TTuple<uint8, uint8, float>> AllCandidateEdges;
	for (int i = 0 ; i< NodeCount ; i++)
	{
		for (int j = 0 ; j < NodeCount ; j++)
		{
			if (i == j) continue;

			float Distance = FVector2D::Distance(DungeonGraph.Nodes[i].UICoord,DungeonGraph.Nodes[j].UICoord);
	
			AllCandidateEdges.Add(MakeTuple(i,j,Distance));
		}
	}
	
	AllCandidateEdges.Sort([](const TTuple<uint8, uint8, float>& A, const TTuple<uint8, uint8, float>& B)
	{
		return A.Get<2>() < B.Get<2>();
	});
	
	TArray<int16> Parent;
	Parent.Init(-1,NodeCount);

	
	std::function<uint8(uint8)> Find = [&Parent, &Find](uint8 Node) -> uint8
	{
		if (Parent[Node] < 0 ) return Node;
		return Parent[Node] = Find(Parent[Node]);
	};

	std::function<bool(uint8, uint8)> Union = [&Parent, &Find](uint8 A, uint8 B) -> bool
	{
		uint8 RootA = Find(A);
		uint8 RootB = Find(B);

		if (RootA == RootB) return false;
		
		Parent[RootB] = RootA;
		return true;
	};
	
	for (const auto& iter : AllCandidateEdges)
	{
		uint8 NodeA = iter.Get<0>();
		uint8 NodeB = iter.Get<1>();

		if (Union(NodeA, NodeB))
		{
			DungeonGraph.Nodes[NodeA].ConnectedNodeIDs.Add(NodeB);
			DungeonGraph.Nodes[NodeB].ConnectedNodeIDs.Add(NodeA);
		}
	}
}

bool UMJDungeonGenerationSubSystem::CheckHasIterableGraph()
{
	const uint8 NodeCount = DungeonGraph.Nodes.Num();
	const uint8 BossID    = DungeonGraph.BossNodeID;
	const uint8 StartID   = DungeonGraph.StartNodeID;
	
	TArray<bool> Visited;
	Visited.Init(false, NodeCount);
	
	TQueue<uint8> Queue;
	Queue.Enqueue(StartID);

	while (!Queue.IsEmpty())
	{
		uint8 Cur = 0;
		if (Queue.Dequeue(Cur))
		{
			for (auto Iter : DungeonGraph.Nodes[Cur].ConnectedNodeIDs)
			{
				if (BossID == Iter)
				{
					continue;
				}
				if (!Visited[Iter])
				{
					Visited[Iter] = true;
					Queue.Enqueue(Iter);
				}
			}
		}

	}
	
	for (uint8 NodeID = 0; NodeID < NodeCount; ++NodeID)
	{
		if (NodeID == BossID) 
			continue;

		if (!Visited[NodeID])
			return false;  
	}

	return true;  
}


bool UMJDungeonGenerationSubSystem::CheckHasRoute(uint8 CurrentNodeNum, uint8 DestNodeNum)
{
	for (auto& iter:DungeonGraph.Nodes[CurrentNodeNum].ConnectedNodeIDs)
	{
		if (iter == DestNodeNum)
		{
			return true;
		}
	}
	return false;
}

FVector2D UMJDungeonGenerationSubSystem::GetQuadBezier(float t, const FVector2D StartPoint, const FVector2D EndPoint,
	const FVector2D ControlPoint)
{
	return pow((1-t),2) * StartPoint + 2 * (1-t) * t * ControlPoint + pow(t,2) * EndPoint; 
}

uint8 UMJDungeonGenerationSubSystem::GetMaxNodeNum() const
{
	return MaxNodeNum;
}

void UMJDungeonGenerationSubSystem::SetMaxNodeNum(uint8 NewMaxNodeNum)
{
	MaxNodeNum = NewMaxNodeNum;
}