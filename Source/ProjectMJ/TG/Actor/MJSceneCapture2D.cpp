// ThenOneDayStudio


#include "TG/Actor/MJSceneCapture2D.h"

#include "MJPortalToDungeon.h"
#include "NavigationSystem.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "NavMesh/RecastNavMesh.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Character.h"
#include "TG/Component/MJMiniMapIconMeshComponent.h"
#include "TG/GameMode/MJGameModeTown.h"
#include "TG/GameState/MJGameStateDungeon.h"


AMJSceneCapture2D::AMJSceneCapture2D()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraHeight = 3000.f;
	
	SetActorLocation(FVector(0.0f,0.0f,CameraHeight));
	SetActorRotation(FRotator3d(-90.0f,0.0f,0.0f));
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMeshComponent"));
	//ProceduralMeshComponent->SetupAttachment(RootComponent);
	
	ProceduralMeshComponent->bCastDynamicShadow = false;
}

void AMJSceneCapture2D::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	AMJGameStateDungeon* GSDungeon = Cast<AMJGameStateDungeon>(UGameplayStatics::GetGameState(GetWorld()));
	if (GSDungeon)
	{
		GSDungeon->OnActorSpawned.AddDynamic(this, &AMJSceneCapture2D::OnActorSpawned);
	}
}

void AMJSceneCapture2D::BeginPlay()
{
	Super::BeginPlay();	
	
	if (Cast<AMJGameModeTown>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(),AMJInteractableActor::StaticClass(), OutActors);

		for (const auto& Iter : OutActors)
		{
			if (IsValid(Iter))
			{
				GetCaptureComponent2D()->ShowOnlyComponents.Add(Iter->FindComponentByClass<UMJMiniMapIconMeshComponent>());
			}
		}
	}
	

	Player = TWeakObjectPtr<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
	if (Player.IsValid())
	{
		UPrimitiveComponent* PlayerMiniMapMeshComp = Cast<UPrimitiveComponent>(Player->FindComponentByClass<UMJMiniMapIconMeshComponent>());
		if (PlayerMiniMapMeshComp)
		{
			GetCaptureComponent2D()->ShowOnlyComponents.Add(PlayerMiniMapMeshComp);
		}
	}
	
	if (MiniMapBackgroundActor)
	{
		GetCaptureComponent2D()->ShowOnlyActors.Add(MiniMapBackgroundActor);
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		APawn* Pawn = Cast<APawn>(Player);
		if (Pawn)
		{
			FNavAgentProperties AgentProps = Cast<APawn>(Player)->GetNavAgentPropertiesRef();
			ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavSys->GetNavDataForProps(AgentProps));
			if (NavMesh)
			{	
				int32 TileCount = NavMesh->GetNavMeshTilesCount();
				
				TArray<FVector> AllVertices;
				TArray<int32> AllTriangles;
				TArray<FColor> AllColors; 

				int32 VertexOffset = 0;

				for (int i = 0; i < TileCount; ++i)
				{
					TArray<FNavPoly> Polys;	
					NavMesh->GetPolysInTile(i, Polys);

					for (const FNavPoly& Poly : Polys)
					{
						TArray<FVector> Verts;
						if (!NavMesh->GetPolyVerts(Poly.Ref, Verts)) continue;

						int NumVerts = Verts.Num();
						
						for (const FVector& V : Verts)
						{
							AllVertices.Add(V);
							AllColors.Add(FColor::Green);
						}

						
						for (int32 v = 1; v < NumVerts - 1; ++v)
						{
							AllTriangles.Add(VertexOffset + 0);
							AllTriangles.Add(VertexOffset + v);
							AllTriangles.Add(VertexOffset + v + 1);
						}
						VertexOffset += NumVerts;
					}
				}

				TArray<FVector> Normals;
				Normals.Init(FVector::UpVector, AllVertices.Num());
				TArray<FVector2D> UV0;
				UV0.Init(FVector2D::ZeroVector, AllVertices.Num());
				TArray<FProcMeshTangent> Tangents;
				Tangents.Init(FProcMeshTangent(), AllVertices.Num());

				ProceduralMeshComponent->CreateMeshSection(
					0,
					AllVertices,
					AllTriangles,
					Normals,
					UV0,
					AllColors,
					Tangents,
					true
				);
				ProceduralMeshComponent->SetMaterial(0,ProceduralMeshMaterial);
				ProceduralMeshComponent->SetVisibility(true);
				GetCaptureComponent2D()->ShowOnlyComponents.Add(ProceduralMeshComponent);
			}
		}
	}
}

void AMJSceneCapture2D::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Player.IsValid())
	{
		FVector3d PlayerLoc = Player->GetActorLocation();

		SetActorLocation(FVector3d(PlayerLoc.X,PlayerLoc.Y, CameraHeight));
	}
}

void AMJSceneCapture2D::OnActorSpawned(AActor* InputActor)
{
	if (IsValid(InputActor))
	{
		UMJMiniMapIconMeshComponent* MiniMapIconMeshComponent = InputActor->FindComponentByClass<UMJMiniMapIconMeshComponent>();
		if (IsValid(MiniMapIconMeshComponent))
		{
			GetCaptureComponent2D()->ShowOnlyComponents.Add(MiniMapIconMeshComponent);

			InputActor->OnDestroyed.AddDynamic(this, &AMJSceneCapture2D::OnActorDestroyed);
			
		}
	}	
}

void AMJSceneCapture2D::OnActorDestroyed(AActor* InputActor)
{
	if (IsValid(InputActor))
	{
		UMJMiniMapIconMeshComponent* MiniMapIconMesh = Cast<UMJMiniMapIconMeshComponent>(InputActor->FindComponentByClass<UMJMiniMapIconMeshComponent>());
		if (IsValid(MiniMapIconMesh))
		{
			GetCaptureComponent2D()->ShowOnlyComponents.Remove(MiniMapIconMesh);
		}
	}	
}

