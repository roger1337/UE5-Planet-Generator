// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcGen/PlanetChunkComponent.h"
#include "ProcGen/ProceduralPlanet.h"
#include "ProcGen/PlanetChunk.h"
#include "ProcGen/ChunkData.h"
#include "ProcGen/NoiseGenerator.h"
#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "Mesh/RealtimeMeshBasicShapeTools.h"
#include "Mesh/RealtimeMeshBuilder.h"
#include "Mesh/RealtimeMeshSimpleData.h"
#include "FastNoiseWrapper.h"
#include "KismetProceduralMeshLibrary.h"

constexpr int Radius = 30000000;
constexpr int ChunkComplexity = 5;
constexpr int MaxSubdivisions = 14;

// Sets default values
AProceduralPlanet::AProceduralPlanet()
{
	NoiseOne = CreateDefaultSubobject<UFastNoiseWrapper>(TEXT("Noise One"));
	NoiseTwo = CreateDefaultSubobject<UFastNoiseWrapper>(TEXT("Noise Two"));
	NoiseThree = CreateDefaultSubobject<UFastNoiseWrapper>(TEXT("Noise Three"));
	NoiseFour = CreateDefaultSubobject<UFastNoiseWrapper>(TEXT("Noise Four"));
	NoiseFive = CreateDefaultSubobject<UFastNoiseWrapper>(TEXT("Noise Five"));


	NoiseGen = NoiseGenerator{};
	NoiseGen.BaseNoise = NoiseOne;
	NoiseGen.BillowRidgeNoise = NoiseTwo;
	NoiseGen.BillowRidgeNoiseDeterminer = NoiseThree;
	NoiseGen.SetupNoise();


	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bAllowTickBeforeBeginPlay = false;


}

// Called when the game starts or when spawned
void AProceduralPlanet::BeginPlay()
{
	Super::BeginPlay();

	TArray<FVector> NormalizedVertices;

	for (FVector Vertex : Icosahedron::IcosahedronVertices) {
		Vertex.Normalize();
		NormalizedVertices.Add(Vertex);
	}

	PlanetVertices.Append(NormalizedVertices);
	PlanetUVs.Append(Icosahedron::UVs);

	for (FVector& v : PlanetVertices) {
		float Noise{ NoiseGen.GetNoise(v.X, v.Y,v.Z) };
		v *= 1 + Noise;
	}

	for (int i = 0; i < 20; i++) {
		ActiveChunks.Add(PlanetChunk{ Icosahedron::IcosahedronTriangles[i], Icosahedron::UVTriangles[i], this, Radius * 3});
	};

	for (PlanetChunk& Chunk : ActiveChunks) {
		for (int _ = 0; _ < ChunkComplexity; _++) {
			Chunk.SubdivideChunkData(Chunk.GetData());
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Vertices Amount %d"), PlanetVertices.Num()));

	for (PlanetChunk& Chunk : ActiveChunks) {
		CreateMesh(Chunk);
	}
}


APlanetChunkComponent* AProceduralPlanet::CreateMesh(PlanetChunk& Chunk) {

	if (MeshSectionMap.Contains(Chunk.GetData()))
		return *MeshSectionMap.Find(Chunk.GetData());

	TArray<Triangle> Triangles;

	TArray<FVector3f> Vertices;
	TMap<int, int> VerticesIndexMap;
	TArray<FVector2D> UVs;

	TArray<ChunkData*> Ancestors{ Chunk.GetData()->GetLowestAncestors(ChunkComplexity) };


	for (ChunkData* LowestAncestor : Ancestors) {
		Triangle Tri;


		for (int i = 0; i < 3; i++) {
			int Idx{ VerticesIndexMap.FindOrAdd(LowestAncestor->TriVerts[i], Vertices.Num()) };
			if (Idx == Vertices.Num()) {
				Vertices.Add(FVector3f{ PlanetVertices[LowestAncestor->TriVerts[i]] * Radius });
				UVs.Add(PlanetUVs[LowestAncestor->UVs[i]]);
			}

			Tri.Vert[i] = Idx;
		}


		Triangles.Add(Tri);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Triangle Amount %d"), Triangles.Num()));

	return MeshSectionMap.Add(Chunk.GetData(), CreateMesh(Chunk.GetData(), Vertices, Triangles, UVs, VerticesIndexMap, Chunk.GetData()->LevelOfSubdivision));
}


void AProceduralPlanet::AddNormalsToFixEdges(ChunkData* Data, TArray<FVector3f>& Vertices, TArray<Triangle>& Triangles, TArray<FVector3f>& Normals, TMap<int, int>& VerticesIndexMap) {
	ChunkData* Neighbours[3];
	Neighbours[0] = nullptr;
	Neighbours[1] = nullptr;
	Neighbours[2] = nullptr;

	if (Data->Parent == nullptr)
		return;
	
	int ChildIdx = -1;
	for (int i = 0; i < 4; i++) {
		if (Data->Parent->Children[i] == Data) {
			ChildIdx = i;
			break;
		}
	}

	if (ChildIdx == 3) {
		for (int i = 0; i < 3; i++) {
			Neighbours[i] = Data->Parent->Children[i];
		}
	}
	else {
		double start = FPlatformTime::Seconds();

		Neighbours[0] = Data->Parent->Children[3];
		TFunction<bool(ChunkData*, ChunkData*, int&)> CheckIsNeighbour{
				[](ChunkData* Data, ChunkData* PlanetChunkData, int& VertexMatches) -> bool {
					if (PlanetChunkData == nullptr)
						return false;

					if (PlanetChunkData == Data || PlanetChunkData == Data->Parent || PlanetChunkData == Data->Parent->Children[3])
						return false;

					int Matches{ 0 };
					for (int a = 0; a < 3; a++) {
						for (int b = 0; b < 3; b++) {

							if (Data->TriVerts[a] == PlanetChunkData->TriVerts[b]) {
								Matches++;
							}

							if (Matches >= 2) {
								VertexMatches = 2;
								return true;
							}
						}
					}

					VertexMatches = Matches;
					return false;
				}
		};

		int NeighbourIndex = 0;
		for (PlanetChunk& Chunk : ActiveChunks) {
			ChunkData* PlanetChunkData{ Chunk.GetData() };

			int VertexMatches{ 0 };
			if (CheckIsNeighbour(Data, PlanetChunkData, VertexMatches)) {
				Neighbours[++NeighbourIndex] = PlanetChunkData;
			}

			if (VertexMatches == 1) { // this means its a bigger chunk that is adjacent. We only check its direct children because we assume that adjacent chunks should only differ in complexity by 1. If it is more, then a seam is ok.
				for (int i = 0; i < 3; i++) {
					if (CheckIsNeighbour(Data, PlanetChunkData->Children[i], VertexMatches)) {
						Neighbours[++NeighbourIndex] = PlanetChunkData->Children[i];
						break;
					}
				}
			}

			if (NeighbourIndex >= 2) {
				break;
			}
		}
		double end = FPlatformTime::Seconds();
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("The neighbours calculation executed in %f seconds."), end - start));

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Neighbours: %d"), NeighbourIndex));
	}


	
	double start = FPlatformTime::Seconds();


	for (ChunkData* Neighbour : Neighbours) {
		if (Neighbour == nullptr)
			continue;




		if (Neighbour->GetDepth() < Data->GetDepth()) {
			PlanetChunk FakeChunk { Neighbour, this, 0 };
			FakeChunk.SubdivideChunkData(FakeChunk.GetData());

		}


		for (ChunkData* LowestAncestor : Neighbour->GetLowestAncestors()) {
			for (int i = 0; i < 3; i++) {
				int TriIdx{ LowestAncestor->TriVerts[i] };

				int* RealIdx = VerticesIndexMap.Find(TriIdx);

				if (RealIdx != nullptr) {
					Triangle Triangle;
					Triangle.Vert[0] = LowestAncestor->TriVerts[0];
					Triangle.Vert[1] = LowestAncestor->TriVerts[1];
					Triangle.Vert[2] = LowestAncestor->TriVerts[2];

					FVector3d P{ FVector3d::CrossProduct(PlanetVertices[Triangle.Vert[1]] * Radius - PlanetVertices[Triangle.Vert[0]] * Radius, PlanetVertices[Triangle.Vert[2]] * Radius - PlanetVertices[Triangle.Vert[0]] * Radius) };
					//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Adding at: %d in I %d The Vector: %f %f %f"), *RealIdx, i, P.X, P.Y, P.Z));


					Normals[*RealIdx] += FVector3f{ P };
				}
			}
		}
	}

	double end = FPlatformTime::Seconds();

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Adding neighbours executed in %f seconds."), end - start));


}

APlanetChunkComponent* AProceduralPlanet::CreateMesh(ChunkData* Data, TArray<FVector3f>& Vertices, TArray<Triangle>& Triangles, TArray<FVector2D>& UVs, TMap<int, int>& VerticesIndexMap, int LevelOfSubdivision) {
	double start = FPlatformTime::Seconds();
	static int SectionIndex = 0;

	FRealtimeMeshStreamSet StreamSet;
	TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2f, 1> Builder{ StreamSet };
	Builder.EnableTangents();
	Builder.EnableTexCoords();
	Builder.EnablePolyGroups();
	Builder.EnableColors();


	TArray<FVector3f> Normals;
	Normals.SetNumZeroed(Vertices.Num());

	for (Triangle& Triangle : Triangles) {
		FVector3f P{ FVector3f::CrossProduct(Vertices[Triangle.Vert[1]] - Vertices[Triangle.Vert[0]], Vertices[Triangle.Vert[2]] - Vertices[Triangle.Vert[0]]) };

		Normals[Triangle.Vert[0]] += P;
		Normals[Triangle.Vert[1]] += P;
		Normals[Triangle.Vert[2]] += P;
	}

	AddNormalsToFixEdges(Data, Vertices, Triangles, Normals, VerticesIndexMap);

	for (FVector3f& Normal : Normals) {
		Normal.Normalize();
		Normal *= -1;
	}



	TArray<FVector3f> TangentsX;
	TArray<FVector3f> TangentsZ;
	TangentsX.SetNumZeroed(Vertices.Num());
	TangentsZ.SetNumZeroed(Vertices.Num());


	TConstArrayView<int> TriangleView { (int*)Triangles.GetData(), Triangles.Num() * 3 };
	TConstArrayView<FVector3f> VerticesView { (FVector3f*)Vertices.GetData(), Vertices.Num() };
	TFunction<FVector2f(int32)> UVGetter{ [&UVs](int32 idx) {
			return FVector2f { UVs[idx] };
		}
	};
	TFunctionRef<void(int32, FVector3f, FVector3f)> TangentsSetter{ [&TangentsX, &TangentsZ](int32 idx, FVector3f TangentX, FVector3f TangentZ) {
			TangentsX[idx] = TangentX;
			TangentsZ[idx] = TangentZ;
		}
	};


	RealtimeMeshAlgo::GenerateTangents(TriangleView, VerticesView, UVGetter, TangentsSetter, true);

	for (int i = 0 ;FVector3f& Vertex : Vertices) {

		FVector3f& Normal{ Normals[i] };
		FVector2D UV{ UVs[i] * 3000 };
		FVector3f Tangent{ TangentsX[i] };


		Builder.AddVertex(FVector3f{ Vertex })
			.SetTexCoords(UV)
			.SetNormalAndTangent(FVector3f{ Normal }, Tangent);
	
		i++;
	}

	for (Triangle& Tri : Triangles) {
		Builder.AddTriangle(Tri.Vert[0], Tri.Vert[1], Tri.Vert[2], 0);
	}

	FRealtimeMeshSectionGroupKey Key = FRealtimeMeshSectionGroupKey::Create(0, FName(FString::Printf(TEXT("%d"), ++SectionIndex)));
	FActorSpawnParameters Params;
	Params.Name = FName(FString::Printf(TEXT("Chunk Number %d"), ++SectionIndex));
	Params.Owner = this;
	Params.bNoFail = true;
	APlanetChunkComponent* ChunkComponent { GetWorld()->SpawnActor<APlanetChunkComponent>(GetActorLocation(), GetActorRotation(), Params) };

	ChunkComponent->CreateChunk(StreamSet, Key, Material);
	double end = FPlatformTime::Seconds();
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("code executed in %f seconds."), end - start));

	return ChunkComponent;
}



// Called every frame
void AProceduralPlanet::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	static float PlanetDivisionTime{ 0 };
	PlanetDivisionTime += DeltaTime;

	if (PlanetDivisionTime > 0.0016f) {
		FDateTime startT = FDateTime::UtcNow();
		int32 startMs = startT.GetMillisecond();
		int64 unixStart = startT.ToUnixTimestamp() * 1000 + startMs;
		APlayerController* Controller{ GetWorld()->GetFirstPlayerController() };
		PlanetDivisionTime = 0;

		if (Controller) {

			FVector PlanetLocation = GetActorLocation();
			FVector PlayerLocation = Controller->GetPawn()->GetActorLocation();

			TArray<uint32> IndicesToRemove;
			TArray<PlanetChunk> PlanetChunksToAdd;

			int i = 0;
			for (PlanetChunk& Chunk : ActiveChunks) {
				FVector CentreOfChunk = ((PlanetVertices[Chunk.GetData()->TriVerts[0]] + PlanetVertices[Chunk.GetData()->TriVerts[1]] + PlanetVertices[Chunk.GetData()->TriVerts[2]]) * Radius) / 3.0f + PlanetLocation;
				uint32 Dist = FVector::Dist(PlayerLocation, CentreOfChunk);

				if (Dist < Chunk.GetDistanceToSplit() && Chunk.GetData()->LevelOfSubdivision <= MaxSubdivisions) {
					(*MeshSectionMap.Find(Chunk.GetData()))->SetActorHiddenInGame(true);
					(*MeshSectionMap.Find(Chunk.GetData()))->SetActorEnableCollision(false);

					TArray<PlanetChunk> Subdivided{ Chunk.Subdivide(ChunkComplexity) };

					for (PlanetChunk& Sub : Subdivided) {
						CreateMesh(Sub);
					}
			
					PlanetChunksToAdd.Append(Subdivided);
					IndicesToRemove.Add(i);
					break;
				}

				i++;
			}

			int IndexOffset{ 0 };

			for (uint32 Index : IndicesToRemove) {
				ActiveChunks.RemoveAt(Index - IndexOffset++);
			}

			ActiveChunks.Append(PlanetChunksToAdd);

		}

		FDateTime End = FDateTime::UtcNow();
		int32 EndMs = End.GetMillisecond();
		int64 unixEnd = End.ToUnixTimestamp() * 1000 + EndMs;

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("IT took %d"), unixEnd - unixStart));
	}
}


// This function gets the vertex on faces' edges if it already exists for subdivision.
uint32 AProceduralPlanet::GetEdgeVertex(uint32 VertexOne, uint32 VertexTwo) {
	uint32 a = std::min(VertexOne, VertexTwo);
	uint32 b = std::max(VertexOne, VertexTwo); // sort by min max, so the order of the edge vertices don't matter.
	TPair<uint32, uint32> key{ a, b };
	uint32 Result{ MidpointLookupMap.FindOrAdd(key, PlanetVertices.Num()) }; // Finds cached vertex index, or associates the vertex indices of this edge to the vertex index of the midpoint.

	if (Result == PlanetVertices.Num()) { // Meaning, there was not a pre-existing midpoint for this edge
		FVector Midpoint = PlanetVertices[a] + PlanetVertices[b];
		Midpoint.Normalize();

		float Noise{ NoiseGen.GetNoise(Midpoint.X, Midpoint.Y,Midpoint.Z) };
		Midpoint *= 1+Noise;

		PlanetVertices.Add(Midpoint);
	}

	return Result;
}

uint32 AProceduralPlanet::GetUVEdgeVertex(uint32 VertexOne, uint32 VertexTwo) {
	uint32 a = std::min(VertexOne, VertexTwo);
	uint32 b = std::max(VertexOne, VertexTwo); // sort by min max, so the order of the edge vertices don't matter.
	TPair<uint32, uint32> key{ a, b };
	uint32 Result{ UVMidpointLookupMap.FindOrAdd(key, PlanetUVs.Num()) }; // Finds cached vertex index, or associates the vertex indices of this edge to the vertex index of the midpoint.

	if (Result == PlanetUVs.Num()) { // Meaning, there was not a pre-existing midpoint for this edge
		FVector2D Midpoint = (PlanetUVs[a] + PlanetUVs[b])*0.5;

		PlanetUVs.Add(Midpoint);
	}

	return Result;
}


