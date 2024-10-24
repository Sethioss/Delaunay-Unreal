// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "IndexTypes.h"
#include "Templates/Tuple.h"
#include "ProceduralWorldGenerator.generated.h"

using namespace UE::Math;
using namespace UE::Geometry;
using PrimReadyList = TMap<FVector2d, TArray<FVector2d>>;
using MSTNode = TPair<FVector2d, FVector2d>;
using MST = TArray<MSTNode>;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELAUNAY_EXERCISE_API UProceduralWorldGenerator : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProceduralWorldGenerator();

	UPROPERTY(EditAnywhere)
	int Resolution = 0;

	UPROPERTY(EditAnywhere)
	FVector3d InitialPosition;

	UPROPERTY(EditAnywhere)
	FVector2f MaxPosition;

	UPROPERTY(EditAnywhere)
	FVector2f MinPosition;

	UPROPERTY(EditAnywhere)
	float CubeSize = 40.0f;

	UPROPERTY(EditAnywhere)
	float PathWidth = 150.0f;

	UPROPERTY(EditAnywhere)
	bool bVisualizePoints = false;

	UPROPERTY(EditAnywhere)
	bool bVisualizeDelaunay = false;

	UPROPERTY(EditAnywhere)
	bool bVisualizeVoronoi = false;

	UPROPERTY(EditAnywhere)
	bool bVisualizePrim = false;

	UPROPERTY(EditAnywhere)
	bool bShowLevel = false;

	UPROPERTY(EditAnywhere, Category="Debug")
	TSubclassOf<class ADebugSphereActor> LevelPointMesh;
	
	UPROPERTY(EditAnywhere, Category="Debug")
	TSubclassOf<class ADebugSphereActor> PointVisualisationActor;

	bool bVisualize = false;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	TArray<FVector2d> Points;
	TArray<AActor*> SpawnedActors;

	void SetRandomVerticesPositions(TArray<FVector2d>& OutVertices) const;
	static double GetRandomPos(const float Min, const float Max);

	UFUNCTION(BlueprintCallable)
	void LoadGeneration();

	void VisualizePoints(TArray<FVector2d>& PointsList) const;
	void VisualizeDelaunay(TArray<FIndex3i>& Tris) const;
	void VisualizeVoronoi(TArray<TArray<FVector2d>>& Cells) const;
	void VisualizePrim(TArray<MSTNode>& Nodes) const;

	PrimReadyList MakePrimNodes(const TArray<TArray<FVector2d>>& VoronoiPoints) const;
    MST PrimAlgorithm(const PrimReadyList& PrimList);
	void SiftUp(TArray<TPair<float, TPair<FVector2d, FVector2d>>>& Heap, int32 Index);
	void SiftDown(TArray<TPair<float, TPair<FVector2d, FVector2d>>>& Heap, int32 Index);
	void AddEdgesToHeap(const FVector2d& Point, const TMap<FVector2d, TArray<FVector2d>>& Graph, TArray<TPair<float, TPair<FVector2d, FVector2d>>>& MinHeap, const TSet<FVector2d>& Visited);

	float GetDistance(const FVector2d& Point1, const FVector2d& Point2);

	void GenerateLevel(MST& Mst);
	
	void GeneratePathFromMST(const TArray<MSTNode>& Edges, UWorld* World);

	void SpawnPathCube(const FVector& Location, float Size, UWorld* World);
	

	

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
};
