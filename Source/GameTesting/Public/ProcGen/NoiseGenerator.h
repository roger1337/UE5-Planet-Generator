#pragma once

#include "CoreMinimal.h"

class UFastNoiseWrapper;

class NoiseGenerator {

public:
	UFastNoiseWrapper* BaseNoise;
	UFastNoiseWrapper* BillowRidgeNoise;
	UFastNoiseWrapper* BillowRidgeNoiseDeterminer;

public:
	NoiseGenerator();
	float GetNoise(float x, float y, float z = 0);
	void SetupNoise();
private:
	float GetNoise3D(UFastNoiseWrapper* Noise, FVector3f Pos);

};