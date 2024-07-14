#pragma once

#include "ProceduralPlanet.h"
#include "CoreMinimal.h"



struct ChunkData {


    int TriVerts[3];
    int UVs[3];
    ChunkData* Parent;
    ChunkData* Children[4];
    int LevelOfSubdivision;

    ChunkData(int One, int Two, int Three, int UVOne, int UVTwo, int UVThree, ChunkData* Parent);
    ChunkData() = default;

    TArray<ChunkData*> GetLowestAncestors();
    TArray<ChunkData*> GetLowestAncestors(int Depth);
    TArray<ChunkData*> GetLowestAncestorsOnSide(int side);
    int GetDepth();

private:
    TArray<ChunkData*> GetLowestAncestorsOnSide(int triangleSideOne, int triangleSideTwo);
    int GetDepth(int CurrentDepth);
    TArray<ChunkData*> GetLowestAncestors(int Depth, int StartingSubdivisionLevel);

};
