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

	UE_LOG(LogTemp, Log, TEXT("STEP 1 : Randomly setting points in area"));

	FDelaunay2 Delaunay;
	Delaunay.bValidateEdges = false;
	Delaunay.bAutomaticallyFixEdgesToDuplicateVertices = true;
	Points->Reserve(Resolution);

	SetRandomVerticesPositions(*Polygon);
	
	UE_LOG(LogTemp, Log, TEXT("STEP 2 : Delaunay triangulation"));

	if (Delaunay.Triangulate(*Polygon, Triangles))
	{
		UE_LOG(LogTemp, Log, TEXT("Triangulation completed."));

		UE_LOG(LogTemp, Log, TEXT("STEP 3 : Voronoi computation"));

		TArray<FVector> VoronoiVectors;
		
		for (int i = 0; i < Polygon->GetVertices().Num(); i++)
		{
			FVector NewVec = FVector(Polygon->GetVertices()[i].X, Polygon->GetVertices()[i].Y, 1.0f);
			VoronoiVectors.Add(NewVec);
		}
		
		const TArrayView<const FVector> VoronoiSites(VoronoiVectors);
		
		TVector<double> MinBoxPos = TVector<double>(MinPosition.X, MinPosition.Y, GenerationHeight);
		TVector<double> MaxBoxPos = TVector<double>(MaxPosition.X, MaxPosition.Y, GenerationHeight);
		
		FBox Box = FBox(MinBoxPos, MaxBoxPos);

		FVoronoiDiagram Diagram(VoronoiSites, Box, UE_SMALL_NUMBER);
		FVoronoiComputeHelper Helper = Diagram.GetComputeHelper();
		
		TArray<FVoronoiCellInfo> VoronoiCells;
		Diagram.ComputeAllCells(VoronoiCells);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Delaunay triangulation has failed!"));
		return;
	}
}

void UProceduralWorldGenerator::VisualizeVoronoi(TArray<FVoronoiCellInfo>& Cells) const
{
	for (int i = 0; i < Cells.Num(); i++)
	{
		for (int j = 0; j < Cells[i].Vertices.Num(); j++)
		{
			GetWorld()->SpawnActor<AActor>(VoronoiActor, Cells[i].Vertices[j], FRotator(0, 0, 0));
		}
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
	
		UE_LOG(LogTemp, Log, TEXT("Added point %i at position %f; %f"), i, RandomPosX, RandomPosY);
	
		FActorSpawnParameters SpawnParameters;
		
		GetWorld()->SpawnActor<AActor>(ActorToSpawn, GetOwner()->GetActorLocation() + FVector(RandomPosX, RandomPosY, GenerationHeight), FRotator(0, 0, 0));
	}

	OutPolygon.SetVertices(*Points);
}

double UProceduralWorldGenerator::GetRandomPos(const float Min, const float Max)
{
	return FMath::RandRange(Min, Max);
}

