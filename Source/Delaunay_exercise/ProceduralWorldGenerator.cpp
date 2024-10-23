// Fill out your copyright notice in the Description page of Project Settings.
#include "ProceduralWorldGenerator.h"
#include "CompGeom/Delaunay2.h"
#include "Kismet/GameplayStatics.h"
#include "DebugSphereActor.h"
#include "Components/StaticMeshComponent.h"

FMSTNode::FMSTNode()
{
}

FMSTNode::FMSTNode(const FVector2d& A, const FVector2d& B)
{
	BeginPoint = A;
	EndPoint = B;
}

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

		GetWorld()->SpawnActor<ADebugSphereActor>(ActorToSpawn, GetOwner()->GetActorLocation() + FVector(PointsList[i].X, PointsList[i].Y, GenerationHeight), FRotator(0, 0, 0));
	}
}

void UProceduralWorldGenerator::VisualizeDelaunay(TArray<FIndex3i>& Tris) const
{
	for (int i = 0; i < Tris.Num(); i++)
	{
		FVector VertexA = FVector(Points[Tris[i].A].X, Points[Tris[i].A].Y, GenerationHeight);
		FVector VertexB = FVector(Points[Tris[i].B].X, Points[Tris[i].B].Y, GenerationHeight);
		FVector VertexC = FVector(Points[Tris[i].C].X, Points[Tris[i].C].Y, GenerationHeight);
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
			FVector3d PointA(Cells[i][j].X, Cells[i][j].Y, GenerationHeight);
			int IncrementIndexToShow = (j+1)%Cells[i].Num();
			FVector3d PointB(Cells[i][IncrementIndexToShow].X, Cells[i][IncrementIndexToShow].Y, GenerationHeight);

			DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + PointA, GetOwner()->GetActorLocation() + PointB, FColor::Purple, true, -1.0f, (uint8)0U, 10.0f);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Voronoi algorithm created %i cells"), Cells.Num());

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



#if WITH_EDITOR
void UProceduralWorldGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (bVisualize)
	{
		LoadGeneration();
	}
}
#endif

