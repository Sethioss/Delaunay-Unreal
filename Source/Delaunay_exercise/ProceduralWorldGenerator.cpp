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
	}
		// TODO: if result is not well defined, is there anything more robust we could do here?
		// Perhaps fill based on the winding number of the input mesh? (But more expensive, and we'd have to handle ~coplanar cases as well)
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Delaunay triangulation has failed!"));
		return;
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
		
		GetWorld()->SpawnActor<AActor>(ActorToSpawn, GetOwner()->GetActorLocation() + FVector(RandomPosX, RandomPosY, 1.0f), FRotator(0, 0, 0));
	}

	OutPolygon.SetVertices(*Points);
}

double UProceduralWorldGenerator::GetRandomPos(const float Min, const float Max)
{
	return FMath::RandRange(Min, Max);
}

