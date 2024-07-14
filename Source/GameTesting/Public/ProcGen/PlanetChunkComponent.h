// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshActor.h"
#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "Mesh/RealtimeMeshBasicShapeTools.h"
#include "Mesh/RealtimeMeshBuilder.h"
#include "Mesh/RealtimeMeshSimpleData.h"
#include "PlanetChunkComponent.generated.h"


/**
 * 
 */
UCLASS()
class GAMETESTING_API APlanetChunkComponent : public ARealtimeMeshActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	URealtimeMeshSimple* ChunkMesh;

	UMaterialInterface* Material;
	FRealtimeMeshStreamSet StreamSet;
	FRealtimeMeshSectionGroupKey Key;

	virtual void BeginPlay() override;
	virtual void OnGenerateMesh_Implementation() override;

public:
	void CreateChunk(FRealtimeMeshStreamSet& StreamSet, FRealtimeMeshSectionGroupKey& Key, UMaterialInterface* Material);
	
};
