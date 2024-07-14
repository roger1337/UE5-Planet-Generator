// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealtimeMeshActor.h"
#include "PlanetChunkComponent.h"
#include "PlanetChunk.h"
#include "ChunkData.h"
#include "NoiseGenerator.h"
#include "ProceduralPlanet.generated.h"

struct Triangle
{
	int Vert[3];
};

namespace Icosahedron {
	const double t{ 1.618034 };

	const FVector IcosahedronVertices[] =
	{
		{-1, t, 0},
		{1, t, 0},
		{-1, -t, 0},
		{1, -t, 0},
		{0, -1, t},
		{0, 1, t},
		{0, -1, -t},
		{0, 1, -t},
		{t, 0, -1},
		{t, 0, 1},
		{-t, 0, -1},
		{-t, 0, 1}


		// let indices 1 and 2 be the pinnacles.
	};

	const Triangle IcosahedronTriangles[] =
	{

		// 5 faces around point 0
		{5, 11, 0},
		{1, 5, 0},
		{7, 1, 0},
		{10, 7, 0},
		{11, 10, 0},

		// 5 adjacent faces
		{9, 5, 1},
		{4, 11, 5},
		{2, 10, 11},
		{6, 7, 10},
		{8, 1, 7},

		// 5 faces around point 3
		{4, 9, 3},
		{2, 4, 3},
		{6, 2, 3},
		{8, 6, 3},
		{9, 8, 3},

		// 5 adjacent faces
		{5, 9, 4},
		{11, 4, 2},
		{10, 2, 6},
		{7, 6, 8},
		{1, 8, 9}
	};

	//The number of points horizontally
	const double w = 5.5;
	//The number of points vertically
	const double h = 3.0;

	const FVector2D UVs[] = {
		{0.5 / w, 0},
		{1.5 / w, 0},
		{2.5 / w, 0},
		{3.5 / w, 0},
		{4.5 / w, 0},
		{0, 1 / h},
		{1 / w, 1 / h},
		{2 / w, 1 / h},
		{3 / w, 1 / h},
		{4 / w, 1 / h},
		{5 / w, 1 / h},
		{0.5 / w, 2 / h},
		{1.5 / w, 2 / h},
		{2.5 / w, 2 / h},
		{3.5 / w, 2 / h},
		{4.5 / w, 2 / h},
		{1, 2 / h},
		{1 / w, 1},
		{2 / w, 1},
		{3 / w, 1},
		{4 / w, 1},
		{5 / w, 1}
	};


	const Triangle UVTriangles[]{
		{6, 5, 0}, {7, 6, 1}, {8, 7, 2}, {9, 8, 3}, {10, 9, 4},
		{12, 6, 7}, {11, 5, 6}, {15, 9, 10}, {14, 8, 9}, {13, 7, 8},
		{11, 12, 17}, {15, 16, 21}, {14, 15, 20}, {13, 14, 19}, {12, 13, 18},
		{6, 12, 11}, {10, 16, 15}, {9, 15, 14}, {8, 14, 13}, {7, 13, 12}
	};
}




struct ChunkData;


struct MeshCreationObject {
    int CurrentChunkCreateIndex;
    PlanetChunk ChunksToCreate;
    int MeshSectionIndexToHide;
};


class UProceduralMeshComponent;
class URealtimeMeshSimple;
struct FRealtimeMeshSectionGroupKey;
class UFastNoiseWrapper;
class UMaterialInterface;

UCLASS()
class GAMETESTING_API AProceduralPlanet : public AActor
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    UMaterialInterface* Material;

	NoiseGenerator NoiseGen;

	UPROPERTY()
	UFastNoiseWrapper* NoiseOne;

	UPROPERTY()
	UFastNoiseWrapper* NoiseTwo;

	UPROPERTY()
	UFastNoiseWrapper* NoiseThree;

	UPROPERTY()
	UFastNoiseWrapper* NoiseFour;

	UPROPERTY()
	UFastNoiseWrapper* NoiseFive;
public:
    TArray<FVector> PlanetVertices;
    TArray<FVector2D> PlanetUVs;
    TArray<PlanetChunk> ActiveChunks;
    TMap<ChunkData*, APlanetChunkComponent*> MeshSectionMap;
    TMap<TPair<uint32, uint32>, uint32> MidpointLookupMap;
    TMap<TPair<uint32, uint32>, uint32> UVMidpointLookupMap;

    uint32 GetEdgeVertex(uint32 VertexOne, uint32 VertexTwo);
    uint32 GetUVEdgeVertex(uint32 VertexOne, uint32 VertexTwo);

	
public:	
	// Sets default values for this actor's properties
	AProceduralPlanet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    APlanetChunkComponent* CreateMesh(ChunkData* Data, TArray<FVector3f>& Vertices, TArray<Triangle>& Triangles, TArray<FVector2D>& UVs, TMap<int, int>& VerticesIndexMap, int LevelOfSubdivision);
    APlanetChunkComponent* CreateMesh(PlanetChunk& Chunk);
	void AddNormalsToFixEdges(ChunkData* Data, TArray<FVector3f>& Vertices, TArray<Triangle>& Triangles, TArray<FVector3f>& Normals, TMap<int, int>& VerticesIndexMap);
};
