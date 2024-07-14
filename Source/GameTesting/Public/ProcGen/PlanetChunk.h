#pragma once




struct ChunkData;
class AProceduralPlanet;
struct Triangle;

class PlanetChunk {
    // In essence, the highest "ChunkData" is the bounds of the chunk.
    // When DistanceToSplit is lower than the distance of the player to the center of the chunk, split PlanetChunk into four, and then add new ChunkData to the lowest ChunkDatas.

private:
    uint32 DistanceToSplit;
    ChunkData* Data;
    AProceduralPlanet* Planet;
public:
    PlanetChunk(Triangle Tri, Triangle UVTri, AProceduralPlanet* ProceduralPlanet, uint32 DistanceToSplit);

    PlanetChunk(ChunkData* Data, AProceduralPlanet* ProceduralPlanet, uint32 DistanceToSplit);

public:
    TArray<PlanetChunk, TFixedAllocator<4>> Subdivide(int ChunkComplexity);
    ChunkData* GetData() { return Data; };
    uint32 GetDistanceToSplit() { return DistanceToSplit; };
    void SubdivideChunkData(ChunkData* Data);
    void SubdivideChunkData(ChunkData* Chunk, int TargetChunkComplexity);
};

