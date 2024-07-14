#include "ProcGen/NoiseGenerator.h"
#include "FastNoiseWrapper.h"
#include "CoreMinimal.h"

NoiseGenerator::NoiseGenerator() {
}



void NoiseGenerator::SetupNoise() {
	int Seed = 1337;
	BaseNoise->SetupFastNoise(EFastNoise_NoiseType::Simplex, Seed, 100.0f, EFastNoise_Interp::Quintic, EFastNoise_FractalType::FBM, 3, 2.0f, 0.5f, 0.45f);
	BillowRidgeNoise->SetupFastNoise(EFastNoise_NoiseType::SimplexFractal, Seed+1, 60.0f, EFastNoise_Interp::Quintic, EFastNoise_FractalType::FBM, 7, 2.0f, 0.5f, 0.45f);
	BillowRidgeNoiseDeterminer->SetupFastNoise(EFastNoise_NoiseType::Perlin, Seed + 2, 20.0f, EFastNoise_Interp::Quintic, EFastNoise_FractalType::FBM, 3, 2.0f, 0.2f, 0.45f);
}

float NoiseGenerator::GetNoise(float x, float y, float z) {

	float BillowRidge { BillowRidgeNoise->GetNoise3D(x, y, z) };

	float Ridge{ 1 - FMath::Abs(BillowRidge) };
	float BillowRidgeDeterminerValue { BillowRidgeNoiseDeterminer->GetNoise3D(x, y, z) };

	FVector3f Pos{ x, y, z };
	float dx{ 0.001f * GetNoise3D(BaseNoise, Pos + FVector3f {0.01, 0.02, 0.03}) };
	float dy{ 0.001f * GetNoise3D(BaseNoise, Pos + FVector3f {0.03, 0.02, 0.07}) };

	Pos.X += dx;
	Pos.Y += dy;

	float Noise{ GetNoise3D(BaseNoise, Pos) };
	Noise += Ridge * BillowRidgeDeterminerValue * 10;
	return Noise / 2000;
}

float NoiseGenerator::GetNoise3D(UFastNoiseWrapper* Noise, FVector3f Pos) {
	return Noise->GetNoise3D(Pos.X, Pos.Y, Pos.Z);
}