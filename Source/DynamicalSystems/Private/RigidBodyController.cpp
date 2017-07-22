#include "DynamicalSystems.h"
#include "RigidBodyController.h"

URigidBodyController::URigidBodyController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URigidBodyController::BeginPlay()
{
	Super::BeginPlay();
}

void URigidBodyController::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

    AActor* Actor = GetOwner();
    
    if (Target && Actor) {

        FVector LocationSetpoint = Target->GetActorLocation();
        FVector LocationFeedback = Actor->GetActorLocation();
        
		LocationError = LocationSetpoint - LocationFeedback;
        auto LocationProportional = LocationError;
		LocationIntegral = LocationIntegral + LocationError * DeltaTime;
        auto LocationDerivative = (LocationError - LocationLastError) / DeltaTime;
		LocationLastError = LocationError;
		LocationControl = LocationKp * LocationProportional + LocationKi * LocationIntegral + LocationKd * LocationDerivative;

		FQuat RotationSetpoint = Target->GetActorRotation().Quaternion();
		FQuat RotationFeedback = Actor->GetActorRotation().Quaternion();
		RotationError = FQuat::Error(RotationSetpoint, RotationFeedback);
		auto RotationProportional = RotationError;
		RotationIntegral = RotationIntegral + RotationError * DeltaTime;
		auto RotationDerivative = (RotationError - RotationLastError) / DeltaTime;
		RotationLastError = RotationError;
		RotationControl = RotationKp * RotationProportional + RotationKi * RotationIntegral + RotationKd * RotationDerivative;
        
        UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>();

        if (StaticMesh) {
            StaticMesh->AddForce(LocationControl);
			RotationSetpoint.EnforceShortestArcWith(RotationFeedback);
			FQuat Rotator = RotationSetpoint * RotationFeedback.Inverse();
			FVector RotatorAxis;
			float RotatorAngle;
			Rotator.ToAxisAndAngle(RotatorAxis, RotatorAngle);
			StaticMesh->AddTorque(RotatorAxis * RotationControl * 10000.f);
        }
    }
}

