// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcGen/PlanetChunkComponent.h"
#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "Mesh/RealtimeMeshBasicShapeTools.h"
#include "Mesh/RealtimeMeshBuilder.h"
#include "Mesh/RealtimeMeshSimpleData.h"

void APlanetChunkComponent::OnGenerateMesh_Implementation() {
	Super::OnGenerateMesh_Implementation();

	ChunkMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();


	if(Material != nullptr) 
		ChunkMesh->SetupMaterialSlot(0, "Material, Material", Material);
	else
		ChunkMesh->SetupMaterialSlot(0, "Material, Material");

	ChunkMesh->CreateSectionGroup(Key, StreamSet);
	ChunkMesh->UpdateSectionConfig(FRealtimeMeshSectionKey::CreateForPolyGroup(Key, 0), FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0), true);
}

 void APlanetChunkComponent::BeginPlay() {

}


void APlanetChunkComponent::CreateChunk(FRealtimeMeshStreamSet& Stream, FRealtimeMeshSectionGroupKey& GroupKey, UMaterialInterface* ChunkMaterial) {
	this->StreamSet = MoveTemp(Stream);
	this->Key = GroupKey;
	this->Material = ChunkMaterial;

}
