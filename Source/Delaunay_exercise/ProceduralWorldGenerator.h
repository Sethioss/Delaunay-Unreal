// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "IndexTypes.h"
#include "Polygon2.h"
#include "Voronoi/Voronoi.h"
#include "ProceduralWorldGenerator.generated.h"

using namespace UE::Math;
using namespace UE::Geometry;

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
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FPolygon2f* Polygon = new FPolygon2f();
	TArray<TVector2<float>>* Points = new TArray<TVector2<float>>();
	TArray<FIndex3i>* Triangles;


	void SetRandomVerticesPositions(FPolygon2f& OutPolygon) const;
	static double GetRandomPos(const float Min, const float Max);

	UFUNCTION(BlueprintCallable)
	void LoadGeneration();

	void VisualizePoints(TArray<TVector2<float>>& Points) const;
	void VisualizeDelaunay(TArray<FIndex3i>& Tris) const;
	void VisualizeVoronoi(TArray<TTuple<FVector, FVector>>& Edges) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
