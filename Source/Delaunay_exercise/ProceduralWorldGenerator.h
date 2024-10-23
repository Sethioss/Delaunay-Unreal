// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "IndexTypes.h"
#include "ProceduralWorldGenerator.generated.h"

USTRUCT()
struct FMSTNode
{
	GENERATED_BODY()

	FVector2d BeginPoint;
	FVector2d EndPoint;

	FMSTNode();
	FMSTNode(const FVector2d& A, const FVector2d& B);
};

using namespace UE::Math;
using namespace UE::Geometry;
using PrimReadyList = TMap<FVector2d, TArray<FVector2d>>;

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

	PrimReadyList MakePrimNodes(const TArray<TArray<FVector2d>>& VoronoiPoints) const;

	

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
};
