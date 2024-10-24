// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugSphereActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ADebugSphereActor::ADebugSphereActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

}

// Called when the game starts or when spawned
void ADebugSphereActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADebugSphereActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

