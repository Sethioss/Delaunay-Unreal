// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralWorldGenerator.h"
#include "CompGeom/Delaunay2.h"
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

	LoadGeneration();
}

void UProceduralWorldGenerator::VisualizePoints(TArray<TVector2<float>>& PointsList) const
{
	for (int i = 0; i < PointsList.Num(); i++)
	{
		FActorSpawnParameters SpawnParameters;
		GetWorld()->SpawnActor<AActor>(ActorToSpawn, GetOwner()->GetActorLocation() + FVector(PointsList[i].X, PointsList[i].Y, GenerationHeight), FRotator(0, 0, 0));
	}
}

void UProceduralWorldGenerator::VisualizeDelaunay(TArray<FIndex3i>& Tris) const
{
	for (int i = 0; i < Tris.Num(); i++)
	{
		FVector2f EdgeA = Polygon->GetVertices()[Tris[i].A];
		FVector2f EdgeB = Polygon->GetVertices()[Tris[i].B];
		FVector2f EdgeC = Polygon->GetVertices()[Tris[i].C];

		DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + FVector(EdgeA.X, EdgeA.Y, GenerationHeight), GetOwner()->GetActorLocation() + FVector(EdgeB.X, EdgeB.Y, GenerationHeight), FColor::Blue, true, -1.0f, (uint8)0U, 5.0f);
		DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + FVector(EdgeB.X, EdgeB.Y, GenerationHeight), GetOwner()->GetActorLocation() + FVector(EdgeC.X, EdgeC.Y, GenerationHeight), FColor::Blue, true, -1.0f, (uint8)0U, 5.0f);
		DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + FVector(EdgeC.X, EdgeC.Y, GenerationHeight), GetOwner()->GetActorLocation() + FVector(EdgeA.X, EdgeA.Y, GenerationHeight), FColor::Blue, true, -1.0f, (uint8)0U, 5.0f);
	}
}

void UProceduralWorldGenerator::VisualizeVoronoi(TArray<TTuple<FVector, FVector>>& Edges) const
{
	for (int i = 0; i < Edges.Num(); i++)
	{
		DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation() + Edges[i].Key, GetOwner()->GetActorLocation() + Edges[i].Value, FColor::Purple, true, -1.0f, (uint8)0U, 10.0f);
	}
}

// Called every frame
void UProceduralWorldGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UProceduralWorldGenerator::SetRandomVerticesPositions(FPolygon2f& OutPolygon) const
{	
	for(int i = 0; i < Resolution; i++)
	{
		double RandomPosX = GetRandomPos(MinPosition.X, MaxPosition.X) + InitialPosition.X;
		double RandomPosY = GetRandomPos(MinPosition.Y, MaxPosition.Y) + InitialPosition.Y;

		Points->Add(TVector2<float>(RandomPosX, RandomPosY));
	}

	OutPolygon.SetVertices(*Points);
}

double UProceduralWorldGenerator::GetRandomPos(const float Min, const float Max)
{
	return FMath::RandRange(Min, Max);
}

void UProceduralWorldGenerator::LoadGeneration()
{
	if (Points != nullptr)
	{
		Points->Empty();
	}
	if (Triangles != nullptr)
	{
		Triangles->Empty();
	}

	UE_LOG(LogTemp, Log, TEXT("STEP 1 : Randomly setting points in area"));

	FDelaunay2 Delaunay;
	Delaunay.bValidateEdges = false;
	Delaunay.bAutomaticallyFixEdgesToDuplicateVertices = true;
	Points->Reserve(Resolution);

	SetRandomVerticesPositions(*Polygon);
	VisualizePoints(*Points);

	UE_LOG(LogTemp, Log, TEXT("STEP 2 : Delaunay triangulation"));

	if (Delaunay.Triangulate(*Polygon, Triangles))
	{
		TArray<FIndex3i> Tris = Delaunay.GetTriangles();
		VisualizeDelaunay(Tris);

		UE_LOG(LogTemp, Log, TEXT("Triangulation completed."));
		UE_LOG(LogTemp, Log, TEXT("STEP 3 : Voronoi computation"));

		TArray<FVector> VoronoiVectors;

		for (int i = 0; i < Polygon->GetVertices().Num(); i++)
		{
			FVector NewVec = FVector(Polygon->GetVertices()[i].X, Polygon->GetVertices()[i].Y, GenerationHeight);
			VoronoiVectors.Add(NewVec);
		}

		const TArrayView<const FVector> VoronoiSites(VoronoiVectors);

		TVector<double> MinBoxPos = TVector<double>(MinPosition.X, MinPosition.Y, GenerationHeight);
		TVector<double> MaxBoxPos = TVector<double>(MaxPosition.X, MaxPosition.Y, GenerationHeight);

		FBox Box = FBox(MinBoxPos, MaxBoxPos);

		FVoronoiDiagram Diagram(VoronoiSites, Box, .1f);
		FVoronoiComputeHelper Helper = Diagram.GetComputeHelper();

		TArray<FVoronoiCellInfo> VoronoiCells;
		Diagram.ComputeAllCells(VoronoiCells);

		TArray<TTuple<FVector, FVector>> Edges;
		TArray<int32> CellMember;
		for (int i = 0; i < VoronoiCells.Num(); i++)
		{
			Diagram.ComputeCellEdges(Edges, CellMember);
		}

		VisualizeVoronoi(Edges);

		UE_LOG(LogTemp, Log, TEXT("Voronoi computation completed"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Delaunay triangulation has failed!"));
		return;
	}
}

