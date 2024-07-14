#include "ProcGen/ChunkData.h"


ChunkData::ChunkData(int One, int Two, int Three, int UVOne, int UVTwo, int UVThree, ChunkData* Parent) {
    TriVerts[0] = One;
    TriVerts[1] = Two;
    TriVerts[2] = Three;
    UVs[0] = UVOne;
    UVs[1] = UVTwo;
    UVs[2] = UVThree;

    this->Parent = Parent;

    if (Parent) {
        LevelOfSubdivision = Parent->LevelOfSubdivision + 1;
    }
    else {
        LevelOfSubdivision = 0;
    }



    Children[0] = nullptr;
    Children[1] = nullptr;
    Children[2] = nullptr;
    Children[3] = nullptr;
}


TArray<ChunkData*> ChunkData::GetLowestAncestors() {
	TArray<ChunkData*> LowestAncestors;

	if (Children[0] == nullptr) {
		LowestAncestors.Add(this); // If no children, this is a lowest ancestor
	}
	else {
		for (ChunkData* Child : Children) {
			if (Child != nullptr) {
				LowestAncestors.Append(Child->GetLowestAncestors());
			}
		}
	}

	return LowestAncestors;
}

TArray<ChunkData*> ChunkData::GetLowestAncestors(int Depth) {
	return GetLowestAncestors(Depth, LevelOfSubdivision);
}


TArray<ChunkData*> ChunkData::GetLowestAncestors(int Depth, int StartingSubdivisionLevel) {
	TArray<ChunkData*> LowestAncestors;

	if (Children[0] == nullptr || LevelOfSubdivision == Depth + StartingSubdivisionLevel) {
		LowestAncestors.Add(this); // If no children, this is a lowest ancestor
	}
	else {
		for (ChunkData* Child : Children) {
			if (Child != nullptr) {
				LowestAncestors.Append(Child->GetLowestAncestors(Depth, StartingSubdivisionLevel));
			}
		}
	}

	return LowestAncestors;
}


// 0, 1, 2
TArray<ChunkData*> ChunkData::GetLowestAncestorsOnSide(int side) {
    return GetLowestAncestorsOnSide(side, (side + 1) % 3);
}
TArray<ChunkData*> ChunkData::GetLowestAncestorsOnSide(int TriangleSideOne, int TriangleSideTwo) {
    TArray<ChunkData*> LowestAncestors;



	if (Children[0] == nullptr) {
		LowestAncestors.Add(this); // If no children, this is a lowest ancestor
	}
	else {
        LowestAncestors.Append(Children[TriangleSideOne]->GetLowestAncestorsOnSide((TriangleSideOne), (TriangleSideTwo)));
        LowestAncestors.Append(Children[TriangleSideTwo]->GetLowestAncestorsOnSide((TriangleSideOne), (TriangleSideTwo)));

	}

    return LowestAncestors;
}



int ChunkData::GetDepth() {
	return GetDepth(0);
}


int ChunkData::GetDepth(int CurrentDepth) {
	if (Children[0] == nullptr) {
		return CurrentDepth;
	}
	else {
		return Children[0]->GetDepth(++CurrentDepth);
	}
}