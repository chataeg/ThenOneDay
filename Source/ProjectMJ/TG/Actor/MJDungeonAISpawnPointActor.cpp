
#include "TG/Actor/MJDungeonAISpawnPointActor.h"

AMJDungeonAISpawnPointActor::AMJDungeonAISpawnPointActor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// Only Visible in Editor
	Mesh->SetHiddenInGame(true);
	NumberToSpawn = 0;
	
}

