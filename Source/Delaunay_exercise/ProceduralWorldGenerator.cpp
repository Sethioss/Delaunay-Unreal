// Fill out your copyright notice in the Description page of Project Settings.
#include "ProceduralWorldGenerator.h"
#include "CompGeom/Delaunay2.h"
#include "Kismet/GameplayStatics.h"
#include "DebugSphereActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values for this component's properties
UProceduralWorldGenerator::UProceduralWorldGenerator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UProceduralWorldGenerator::BeginPlay()
{
	Super::BeginPlay();

	bVisualize = true;
	LoadGeneration();
}

void UProceduralWorldGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADebugSphereActor::StaticClass(), FoundActors);

	for (int i = 0; i < FoundActors.Num(); i++)
	{
		GetWorld()->DestroyActor(FoundActors[i]);
	}
}

void UProceduralWorldGenerator::LoadGeneration()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADebugSphereActor::StaticClass(), FoundActors);

	for (int i = 0; i < FoundActors.Num(); i++)
	{
		GetWorld()->DestroyActor(FoundActors[i]);
	}

	for(int i = 0; i < SpawnedActors.Num(); i++)
	{
		GetWorld()->DestroyActor(SpawnedActors[i]);
	}
	SpawnedActors.Reset();

#if WITH_ENGINE || UE_BUILD_DEBUG
	FlushPersistentDebugLines(GetWorld());
#endif

	Points.Reset();

	UE_LOG(LogTemp, Log, TEXT("STEP 1 : Randomly setting points in area"));

	FDelaunay2 Delaunay;
	Delaunay.bValidateEdges = false;
	Delaunay.bAutomaticallyFixEdgesToDuplicateVertices = true;
	
	Points.Reserve(Resolution);
	
	SetRandomVerticesPositions(Points);
	if (bVisualizePoints)
	{
		VisualizePoints(Points);
	}

	UE_LOG(LogTemp, Log, TEXT("STEP 2 : Delaunay triangulation"));

	if (Delaunay.Triangulate(Points))
	{
		TArray<FIndex3i> Tris = Delaunay.GetTriangles();
		if (bVisualizeDelaunay)
		{
			VisualizeDelaunay(Tris);
		}

		UE_LOG(LogTemp, Log, TEXT("Triangulation completed."));
		UE_LOG(LogTemp, Log, TEXT("STEP 3 : Voronoi calculation"));

		TArray<FVector2d> Points2d;
		Points2d.Reserve(Points.Num());

		for(int i = 0; i < Points.Num(); i++)
		{
			FVector2d* FinalPoint = new FVector2d(Points[i].X, Points[i].Y);
			Points2d.Add(*FinalPoint);
		}
		
		TArray<TArray<FVector2d>> VoronoiEdges = Delaunay.GetVoronoiCells(Points2d, false, FAxisAlignedBox2d(FVector2d(MinPosition.X, MinPosition.Y), FVector2d(MaxPosition.X, MaxPosition.Y)));
		if (bVisualizeVoronoi)
		{
			VisualizeVoronoi(VoronoiEdges);
		}

		UE_LOG(LogTemp, Log, TEXT("Voronoi computation completed"));

		PrimReadyList Nodes = MakePrimNodes(VoronoiEdges);
		TArray<MSTNode> FinalMSTNodes = PrimAlgorithm(Nodes);

		if (bVisualizePrim)
		{
			VisualizePrim(FinalMSTNodes);
		}

		if(bShowLevel)
		{
			GenerateLevel(FinalMSTNodes);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Delaunay triangulation has failed!"));
		return;
	}
}

void UProceduralWorldGenerator::SetRandomVerticesPositions(TArray<FVector2d>& OutVertices) const
{	
	for(int i = 0; i < Resolution; i++)
	{
		double RandomPosX = GetRandomPos(MinPosition.X, MaxPosition.X) + InitialPosition.X;
		double RandomPosY = GetRandomPos(MinPosition.Y, MaxPosition.Y) + InitialPosition.Y;

		FVector2d Vertex = FVector2d(RandomPosX, RandomPosY);
		OutVertices.Add(Vertex);
	}
}

double UProceduralWorldGenerator::GetRandomPos(const float Min, const float Max)
{
	return FMath::RandRange(Min, Max);
}

void UProceduralWorldGenerator::VisualizePoints(TArray<FVector2d>& PointsList) const
{
	for (int i = 0; i < PointsList.Num(); i++)
	{
		FActorSpawnParameters SpawnParameters;

		GetWorld()->SpawnActor<ADebugSphereActor>(PointVisualisationActor, GetOwner()->GetActorLocation() + FVector(PointsList[i].X, PointsList[i].Y, 0.0f), FRotator(0, 0, 0));
	}
}

void UProceduralWorldGenerator::VisualizeDelaunay(TArray<FIndex3i>& Tris) const
{
	for (int i = 0; i < Tris.Num(); i++)
	{
		FVector VertexA = FVector(Points[Tris[i].A].X, Points[Tris[i].A].Y, 0.0f);
		FVector VertexB = FVector(Points[Tris[i].B].X, Points[Tris[i].B].Y, 0.0f);
		FVector VertexC = FVector(Points[Tris[i].C].X, Points[Tris[i].C].Y, 0.0f);
		DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + VertexA, GetOwner()->GetActorLocation() + VertexB, FColor::Blue, true, -1.0f, (uint8)0U, 5.0f);
		DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + VertexB, GetOwner()->GetActorLocation() + VertexC, FColor::Blue, true, -1.0f, (uint8)0U, 5.0f);
		DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + VertexC, GetOwner()->GetActorLocation() + VertexA, FColor::Blue, true, -1.0f, (uint8)0U, 5.0f);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Delaunay algorithm has created a polygon with %i triangles"), Tris.Num());
}

void UProceduralWorldGenerator::VisualizeVoronoi(TArray<TArray<FVector2d>>& Cells) const
{
	for (int i = 0; i < Cells.Num(); i++)
	{
		for(int j = 0; j < Cells[i].Num(); j++)
		{
			FVector3d PointA(Cells[i][j].X, Cells[i][j].Y, 0.0f);
			int IncrementIndexToShow = (j+1)%Cells[i].Num();
			FVector3d PointB(Cells[i][IncrementIndexToShow].X, Cells[i][IncrementIndexToShow].Y, 0.0f);

			DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + PointA, GetOwner()->GetActorLocation() + PointB, FColor::Purple, true, -1.0f, (uint8)0U, 10.0f);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Voronoi algorithm created %i cells"), Cells.Num());

}

void UProceduralWorldGenerator::VisualizePrim(TArray<MSTNode>& Nodes) const
{
	for (int i = 0; i < Nodes.Num(); i++)
	{
		FVector3d PointA(Nodes[i].Key.X, Nodes[i].Key.Y, 0.0f);
		FVector3d PointB(Nodes[i].Value.X, Nodes[i].Value.Y, 0.0f);
		
		DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + PointA, GetOwner()->GetActorLocation() + PointB, FColor::Black, true, -1.0f, 0U, 10.0f);
	}
}

PrimReadyList UProceduralWorldGenerator::MakePrimNodes(const TArray<TArray<FVector2d>>& VoronoiPoints) const
{
	PrimReadyList PrimMap;
	
	for(int i = 0; i < VoronoiPoints.Num(); i++)
	{
		for(int j = 0; j < VoronoiPoints[i].Num(); j++)
		{
			int ActualIncrementedValue = (j+1)%VoronoiPoints[i].Num();
			
			if(!PrimMap.Contains(VoronoiPoints[i][j]))
			{
				PrimMap.Add(VoronoiPoints[i][j]);
			}
			PrimMap[VoronoiPoints[i][j]].Add(VoronoiPoints[i][ActualIncrementedValue]);
		}
	}

	return PrimMap;
}

TArray<MSTNode> UProceduralWorldGenerator::PrimAlgorithm(const PrimReadyList& PrimList)
{
	TSet<FVector2d> Visited;
	TArray<TPair<float, TPair<FVector2d, FVector2d>>> MinHeap;
	TArray<MSTNode> NodesToReturn;

	auto StartPointIter = PrimList.CreateConstIterator();
    
	FVector2d StartPoint = StartPointIter.Key();
	Visited.Add(StartPoint);
	
	AddEdgesToHeap(StartPoint, PrimList, MinHeap, Visited);
	
	while (MinHeap.Num() > 0)
	{

		TPair<float, TPair<FVector2d, FVector2d>> MinEdge = MinHeap[0];
		MinHeap.RemoveAt(0);
        
		FVector2d From = MinEdge.Value.Key;
		FVector2d To = MinEdge.Value.Value;
		
		if (Visited.Contains(To))
		{
			continue;
		}
		
		NodesToReturn.Add(TPair<FVector2d, FVector2d>(From, To));
		
		Visited.Add(To);
		
		AddEdgesToHeap(To, PrimList, MinHeap, Visited);
	}

	return NodesToReturn;
}

void UProceduralWorldGenerator::AddEdgesToHeap(const FVector2d& Point, const TMap<FVector2d, TArray<FVector2d>>& Graph, TArray<TPair<float, TPair<FVector2d, FVector2d>>>& MinHeap, const TSet<FVector2d>& Visited)
{
	const TArray<FVector2d>* AdjacentPoints = Graph.Find(Point);
	if (AdjacentPoints)
	{
		for (const FVector2d& Neighbor : *AdjacentPoints)
		{
			if (!Visited.Contains(Neighbor))
			{
				float Weight = GetDistance(Point, Neighbor);
				MinHeap.Add(TPair<float, TPair<FVector2d, FVector2d>>(Weight, TPair<FVector2d, FVector2d>(Point, Neighbor)));
			}
		}
	}
	
	MinHeap.Sort([](const TPair<float, TPair<FVector2d, FVector2d>>& A, const TPair<float, TPair<FVector2d, FVector2d>>& B)
	{
		return A.Key < B.Key;
	});
}

float UProceduralWorldGenerator::GetDistance(const FVector2d& Point1, const FVector2d& Point2)
{
	return FVector2d::Distance(Point1, Point2);
}

void UProceduralWorldGenerator::GenerateLevel(MST& MST)
{
	for(int i = 0; i < MST.Num(); i++)
	{
		FVector VertexPos = FVector(MST[i].Key.X, MST[i].Key.Y, 0.0f);
		GetWorld()->SpawnActor<ADebugSphereActor>(LevelPointMesh, GetOwner()->GetActorLocation() + VertexPos, FRotator(0, 0, 0));
	}
	
	GeneratePathFromMST(MST, GetWorld());
}

void UProceduralWorldGenerator::GeneratePathFromMST(const TArray<MSTNode>& Edges, UWorld* World)
{
    if (!World) return;

    // Parameters for path generation
    const float HalfCube = CubeSize * 0.5f;
    const int32 NumCubesWidth = FMath::Max(1, FMath::RoundToInt(PathWidth / CubeSize));
    
    for (const auto& Edge : Edges)
    {
        // Convert 2D points to 3D (assuming we're working on XY plane with constant Z)
        FVector Start(GetOwner()->GetActorLocation().X + Edge.Key.X, GetOwner()->GetActorLocation().Y + Edge.Key.Y, 0.0f);
        FVector End(GetOwner()->GetActorLocation().X + Edge.Value.X, GetOwner()->GetActorLocation().Y + Edge.Value.Y, 0.0f);
        
        // Calculate direction and length
        FVector Direction = (End - Start);
        float Length = Direction.Size();
        Direction.Normalize();
        
        // Calculate perpendicular vector for path width
        FVector Perpendicular(-Direction.Y, Direction.X, 0.0f);
        
        // Calculate number of cubes needed for length
        int32 NumCubesLength = FMath::Max(1, FMath::RoundToInt(Length / CubeSize));
        
        // Generate grid of cubes
        for (int32 LengthIndex = 0; LengthIndex < NumCubesLength; LengthIndex++)
        {
            float LengthOffset = LengthIndex * CubeSize;
            
            for (int32 WidthIndex = 0; WidthIndex < NumCubesWidth; WidthIndex++)
            {
                // Calculate offset from center of path
                float WidthOffset = (WidthIndex - (NumCubesWidth - 1) * 0.5f) * CubeSize;
                
                // Calculate cube position
                FVector CubePos = Start + Direction * LengthOffset + Perpendicular * WidthOffset;
                
                // Spawn cube
                SpawnPathCube(CubePos, CubeSize, World);
            }
        }
    }
}

void UProceduralWorldGenerator::SpawnPathCube(const FVector& Location, float Size, UWorld* World)
{
    AActor* Cube = World->SpawnActor<AActor>();
    if (!Cube) return;

	SpawnedActors.Add(Cube);
	
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Cube);
    MeshComp->RegisterComponent();
    Cube->SetRootComponent(MeshComp);
	
    if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube")))
    {
        MeshComp->SetStaticMesh(CubeMesh);
    	
        Cube->SetActorScale3D(FVector(Size / 100.0f));
        Cube->SetActorLocation(Location);
    	
        MeshComp->SetMobility(EComponentMobility::Static);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

#if WITH_EDITOR
void UProceduralWorldGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (bVisualize)
	{
		LoadGeneration();
	}
}
#endif

