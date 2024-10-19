// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "IndexTypes.h"
#include "Polygon2.h"
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

	UPROPERTY(EditAnywhere, Category="Debug")
	TSubclassOf<AActor> ActorToSpawn;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FPolygon2f* Polygon;
	TArray<TVector2<float>>* Points;
	TArray<FIndex3i>* Vertices;
	TArray<FIndex2i>* Edges;
	
	void SetRandomVerticesPositions(FPolygon2f& OutPolygon) const;
	static double GetRandomPos(const float Min, const float Max);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
