// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceshipPawnBase.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GenericPlatform/GenericPlatformMath.h"

// Sets default values
ASpaceshipPawnBase::ASpaceshipPawnBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FloatingMovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovementComponent"));
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = Sphere;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpaceshipCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SpaceshipCamera->SetupAttachment(SpringArm);

	SpaceshipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Spaceship Mesh"));

	SpaceshipMesh->SetupAttachment(RootComponent);
	SpringArm->SetupAttachment(RootComponent);
	FloatingMovementComponent->bConstrainToPlane = false;

	SpaceshipMesh->SetUsingAbsoluteRotation(false);

}

// Called when the game starts or when spawned
void ASpaceshipPawnBase::BeginPlay()
{
	SetActorRotation(FRotator{ 0, 180, 0 });
	SpaceshipMesh->SetRelativeRotation(FRotator{ 0, 180, 0 });
	Super::BeginPlay();
	
}

// Called every frame
void ASpaceshipPawnBase::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);


	if (Controller == nullptr)
		return;




	FRotator Rotator{ Controller->GetControlRotation() };


	// make it in the same space as getrelativerotation
	Rotator.Yaw -= 180;
	if (Rotator.Pitch > 270)
		Rotator.Pitch -= 360;





	FRotator CurrentRotator{ SpaceshipMesh->GetRelativeRotation() };



	// instead of 180 to -180 make it 0 to 360
	if (Rotator.Yaw < 0)
		Rotator.Yaw += 360;
	if (CurrentRotator.Yaw < 0)
		CurrentRotator.Yaw += 360;



	float YawDiff = Rotator.Yaw - CurrentRotator.Yaw;
	float PitchDiff = Rotator.Pitch - CurrentRotator.Pitch;

	if (YawDiff > 180) {
		YawDiff -= 360;
	}
	if (YawDiff < -180) {
		YawDiff += 360;
	}





	FVector2D LookDiffVector{ YawDiff, PitchDiff };
	LookDiffVector.Normalize();

	FVector2D DiffVector { LookDiffVector * RotationalSpeed_Deg_s * DeltaTime };

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%f %f %f %f -- %f %f -- %f %f"), CurrentRotator.Yaw, CurrentRotator.Pitch, Rotator.Yaw, Rotator.Pitch, YawDiff, PitchDiff));




	if(CurrentRotator.Yaw != Rotator.Yaw)
		CurrentRotator.Yaw += DiffVector.X;
	if (CurrentRotator.Pitch != Rotator.Pitch)
		CurrentRotator.Pitch += DiffVector.Y;


	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Sign %f %f"), FMath::Sign(DiffVector.X), FMath::Sign(YawDiff)));

	if (!(Rotator.Yaw - CurrentRotator.Yaw > 180 || Rotator.Yaw - CurrentRotator.Yaw < -180) && FMath::Sign(Rotator.Yaw - (DiffVector.X + CurrentRotator.Yaw)) != FMath::Sign(YawDiff))
		CurrentRotator.Yaw = Rotator.Yaw;
	if (FMath::Sign(Rotator.Pitch - (DiffVector.Y + CurrentRotator.Pitch)) != FMath::Sign(PitchDiff))
		CurrentRotator.Pitch = Rotator.Pitch;


	CurrentRotator.Roll = Controller->GetControlRotation().Roll;
	SpaceshipMesh->SetRelativeRotation(CurrentRotator);


}


// Called to bind functionality to input
void ASpaceshipPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input{ Cast<UEnhancedInputComponent>(PlayerInputComponent) };
	// You can bind to any of the trigger events here by changing the "ETriggerEvent" enum value
	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceshipPawnBase::Look);
	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceshipPawnBase::Move);

}


void ASpaceshipPawnBase::Look(const FInputActionValue& Value) {
	FVector2D LookAxisVector{ Value.Get<FVector2D>() };

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}

}

void ASpaceshipPawnBase::Move(const FInputActionValue& Value) {
	FVector2D MoveVector{ Value.Get<FVector2D>() };



	AddMovementInput(SpaceshipMesh->GetForwardVector(), MoveVector.Y, false);
	AddControllerRollInput(MoveVector.X);
}


