// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SpaceshipPawnBase.generated.h"

class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class USphereComponent;
class UFloatingPawnMovement;
class UStaticMeshComponent;
struct FInputActionValue;

UCLASS()
class GAMETESTING_API ASpaceshipPawnBase : public APawn
{
	GENERATED_BODY()

public:

	
	ASpaceshipPawnBase();

	UPROPERTY(EditAnywhere) USphereComponent* Sphere;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh") UStaticMeshComponent* SpaceshipMesh;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship Characteristics") int32 RotationalSpeed_Deg_s;



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement") UFloatingPawnMovement* FloatingMovementComponent;

	UPROPERTY(EditAnywhere, Category = "Camera") UCameraComponent* SpaceshipCamera;
	UPROPERTY(EditAnywhere, Category = "Camera") USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true")) UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true")) UInputAction* MoveAction;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	FRotator normalise(FRotator in);
}; 

