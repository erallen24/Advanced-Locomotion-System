// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/FCFBaseAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"

void UFCFBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<AFluidityCharacter>(TryGetPawnOwner());
	if (Character)
	{
		MovementComponent = Character->GetCharacterMovement();
	}
}

void UFCFBaseAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// Dynamic Caching: Fixes the bug where variables stop updating!
	if (!Character)
	{
		Character = Cast<AFluidityCharacter>(TryGetPawnOwner());
		if (Character)
		{
			MovementComponent = Character->GetCharacterMovement();
		}
	}

	// Abort if we STILL don't have a character
	if (!Character || !MovementComponent || DeltaSeconds == 0.f) return;

	// Execute the update chain exactly like your Blueprint sequence!
	GetCharacterStates();
	GetVelocityData();
	GetAccelerationData();
	GetLocationData();
	GetRotationData(DeltaSeconds);
	UpdateOrientationData();
}

void UFCFBaseAnimInstance::GetCharacterStates()
{
	LastFrameGait = CurrentGait;
	CurrentGait = Character->CurrentGait;
	IsGaitChanged = (CurrentGait != LastFrameGait);
	EquippedGun = Character->EquippedGun;
}

void UFCFBaseAnimInstance::GetVelocityData()
{
	CharacterVelocity = Character->GetVelocity();
	CharacterVelocity2D = CharacterVelocity * FVector(1.f, 1.f, 0.f);
}

void UFCFBaseAnimInstance::GetAccelerationData()
{
	Acceleration = MovementComponent->GetCurrentAcceleration();
	Acceleration2D = Acceleration * FVector(1.f, 1.f, 0.f);
	IsAccelerating = !UKismetMathLibrary::NearlyEqual_FloatFloat(Acceleration2D.Size2D(), 0.f);
}

void UFCFBaseAnimInstance::GetLocationData()
{
	WorldLocation = Character->GetActorLocation();
}

void UFCFBaseAnimInstance::GetRotationData(float DeltaSeconds)
{
	WorldRotation = Character->GetActorRotation();
	LastFrameActorYaw = ActorYaw;
	ActorYaw = WorldRotation.Yaw;
	DeltaActorYaw = ActorYaw - LastFrameActorYaw;

	// Calculate Lean Angle
	float TargetLean = (DeltaActorYaw / DeltaSeconds) / 6.0f;
	float DirectionMultiplier = 0.f;
	if (VelocityLocomotionDirection == ELocomotionDirection::Forward) DirectionMultiplier = 1.f;
	else if (VelocityLocomotionDirection == ELocomotionDirection::Backward) DirectionMultiplier = -1.f;

	LeanAngle = UKismetMathLibrary::ClampAngle(TargetLean * DirectionMultiplier, -90.f, 90.f);
}

void UFCFBaseAnimInstance::UpdateOrientationData()
{
	LastFrameVelocityLocomotionDirection = VelocityLocomotionDirection;
	VelocityLocomotionAngle = UKismetAnimationLibrary::CalculateDirection(CharacterVelocity2D, WorldRotation);
	VelocityLocomotionDirection = CalculateLocomotionDirection(VelocityLocomotionAngle, -50.f, 50.f, -130.f, 130.f, 20.f, VelocityLocomotionDirection);
}

ELocomotionDirection UFCFBaseAnimInstance::CalculateLocomotionDirection(float Angle, float ForwardMin, float ForwardMax, float BackwardMin, float BackwardMax, float Deadzone, ELocomotionDirection CurrentDir)
{
	// 1. If we are already moving Forward/Backward, we EXPAND the range (+/- Deadzone)
	// This makes the animation "sticky" so slight joystick wobbles don't break the run.
	if (CurrentDir == ELocomotionDirection::Forward || CurrentDir == ELocomotionDirection::Backward)
	{
		if (UKismetMathLibrary::InRange_FloatFloat(Angle, ForwardMin - Deadzone, ForwardMax + Deadzone)) return ELocomotionDirection::Forward;
		if (UKismetMathLibrary::InRange_FloatFloat(Angle, BackwardMin - Deadzone, BackwardMax + Deadzone)) return ELocomotionDirection::Backward;
	}
	// 2. If we are moving Left/Right, we SHRINK the range (+/- Deadzone inverted)
	// This forces the player to intentionally "commit" to a forward/backward angle to snap out of the strafe.
	else
	{
		if (UKismetMathLibrary::InRange_FloatFloat(Angle, ForwardMin + Deadzone, ForwardMax - Deadzone)) return ELocomotionDirection::Forward;
		if (UKismetMathLibrary::InRange_FloatFloat(Angle, BackwardMin + Deadzone, BackwardMax - Deadzone)) return ELocomotionDirection::Backward;
	}

	// 3. Fallback: If we aren't inside any Forward/Backward ranges, we must be strafing.
	if (Angle >= 0.f) return ELocomotionDirection::Right;

	return ELocomotionDirection::Left;
}


