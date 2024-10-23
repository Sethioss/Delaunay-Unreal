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
using FMSTNode = TPair<FVector2d, FVector2d>;

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
	bool bVisualizePoints = false;

	UPROPERTY(EditAnywhere)
	bool bVisualizeDelaunay = false;

	UPROPERTY(EditAnywhere)
	bool bVisualizeVoronoi = false;

	UPROPERTY(EditAnywhere)
	bool bVisualizePrim = false;
	
	UPROPERTY(EditAnywhere)
	float GenerationHeight = 1.0f;

	UPROPERTY(EditAnywhere, Category="Debug")
	TSubclassOf<class ADebugSphereActor> ActorToSpawn;

	bool bVisualize = false;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	TArray<FVector2d> Points;

	void SetRandomVerticesPositions(TArray<FVector2d>& OutVertices) const;
	static double GetRandomPos(const float Min, const float Max);

	UFUNCTION(BlueprintCallable)
	void LoadGeneration();

	void VisualizePoints(TArray<FVector2d>& PointsList) const;
	void VisualizeDelaunay(TArray<FIndex3i>& Tris) const;
	void VisualizeVoronoi(TArray<TArray<FVector2d>>& Cells) const;
	void VisualizePrim(TArray<FMSTNode>& Nodes) const;

	PrimReadyList MakePrimNodes(const TArray<TArray<FVector2d>>& VoronoiPoints) const;
    TArray<FMSTNode> PrimAlgorithm(const PrimReadyList& PrimList);
	void AddEdgesToHeap(const FVector2d& Point, const TMap<FVector2d, TArray<FVector2d>>& Graph, TArray<TPair<float, TPair<FVector2d, FVector2d>>>& MinHeap, const TSet<FVector2d>& Visited);

	float GetDistance(const FVector2d& Point1, const FVector2d& Point2);
	

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
};
