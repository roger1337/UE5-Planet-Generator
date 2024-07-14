#include "ProcGen/PlanetChunk.h"
#include "ProcGen/ProceduralPlanet.h"


PlanetChunk::PlanetChunk(Triangle Tri, Triangle UVTri, AProceduralPlanet* ProceduralPlanet, uint32 DistanceToSplit) : Data{ new ChunkData() } {
	Data->TriVerts[0] = Tri.Vert[0];
	Data->TriVerts[1] = Tri.Vert[1];
	Data->TriVerts[2] = Tri.Vert[2];
	Data->UVs[0] = UVTri.Vert[0];
	Data->UVs[1] = UVTri.Vert[1];
	Data->UVs[2] = UVTri.Vert[2];

	Planet = ProceduralPlanet;
	this->DistanceToSplit = DistanceToSplit;

}

PlanetChunk::PlanetChunk(ChunkData* Data, AProceduralPlanet* ProceduralPlanet, uint32 DistanceToSplit) {
	this->Data = Data;
	Planet = ProceduralPlanet;
	this->DistanceToSplit = DistanceToSplit;
}


TArray<PlanetChunk, TFixedAllocator<4>> PlanetChunk::Subdivide(int ChunkComplexity) {
	TArray<PlanetChunk, TFixedAllocator<4>> NewPlanetChunks;

	PlanetChunk::SubdivideChunkData(Data, ChunkComplexity);

	NewPlanetChunks.Add(PlanetChunk{ Data->Children[0], Planet, DistanceToSplit / 2 });
	NewPlanetChunks.Add(PlanetChunk{ Data->Children[1], Planet, DistanceToSplit / 2 });
	NewPlanetChunks.Add(PlanetChunk{ Data->Children[2], Planet, DistanceToSplit / 2 });
	NewPlanetChunks.Add(PlanetChunk{ Data->Children[3], Planet, DistanceToSplit / 2 });

	return NewPlanetChunks;
}


// Make sure order is correct
void PlanetChunk::SubdivideChunkData(ChunkData* Chunk) {
	SubdivideChunkData(Chunk, 999);
}

void PlanetChunk::SubdivideChunkData(ChunkData* Chunk, int TargetChunkComplexity) {
	if (Chunk == nullptr) {
		return;
	}



	TArray<ChunkData*> LowestAncestors{ Chunk->GetLowestAncestors() };

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Amount %d"), LowestAncestors.Num()));

	for (ChunkData* Child : LowestAncestors) {

		if (Child->LevelOfSubdivision - Chunk->LevelOfSubdivision > TargetChunkComplexity)
			continue;



		int Midpoints[3];
		int UVMidpoints[3];

		for (int Edge = 0; Edge < 3; ++Edge) {
			Midpoints[Edge] = Planet->GetEdgeVertex(Child->TriVerts[Edge], Child->TriVerts[(Edge + 1) % 3]); // Populate the midpoint array of the current triangle. Retrieve or cache existing midpoints on triangle edges.
		}

		for (int Edge = 0; Edge < 3; ++Edge) {
			UVMidpoints[Edge] = Planet->GetUVEdgeVertex(Child->UVs[Edge], Child->UVs[(Edge + 1) % 3]); // Populate the midpoint array of the current triangle. Retrieve or cache existing midpoints on triangle edges.
		}




		Child->Children[0] = new ChunkData{ Child->TriVerts[0], Midpoints[0], Midpoints[2], Child->UVs[0], UVMidpoints[0], UVMidpoints[2], Child };
		Child->Children[1] = new ChunkData{ Midpoints[0], Child->TriVerts[1], Midpoints[1], UVMidpoints[0], Child->UVs[1], UVMidpoints[1], Child };
		Child->Children[2] = new ChunkData{ Midpoints[2], Midpoints[1], Child->TriVerts[2], UVMidpoints[2], UVMidpoints[1], Child->UVs[2], Child };
		Child->Children[3] = new ChunkData{ Midpoints[0], Midpoints[1], Midpoints[2],  UVMidpoints[0], UVMidpoints[1], UVMidpoints[2], Child };

	}
}