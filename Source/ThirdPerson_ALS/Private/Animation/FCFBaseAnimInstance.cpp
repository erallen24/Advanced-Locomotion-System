// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/FCFBaseAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AnimCharacterMovementLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
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

void UFCFBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// We MUST run debugging on the Game Thread, otherwise DrawDebug will crash the engine!
#if WITH_EDITOR
	DrawDebug();
#endif
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

	GaitValue = (CurrentGait == EGaitState::Walk) ? 0.2f : 1.0f;
}

void UFCFBaseAnimInstance::GetVelocityData()
{
	if (!MovementComponent) return;

	FVector Velocity = MovementComponent->Velocity;
	CharacterVelocity2D = FVector(Velocity.X, Velocity.Y, 0.f);

	// 1. Calculate Locomotion Speed for Stride Warping
	// (This replaces the VSizeXY node in your Blueprint!)
	LocomotionSpeed = CharacterVelocity2D.Size2D();

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
	if (!Character) return;

	WorldRotation = Character->GetActorRotation();
	LastFrameActorYaw = ActorYaw;
	ActorYaw = WorldRotation.Yaw;
	DeltaActorYaw = ActorYaw - LastFrameActorYaw;

	if (LocomotionSpeed > 3.f)
	{
		VelocityLocomotionAngle = UKismetAnimationLibrary::CalculateDirection(CharacterVelocity2D, Character->GetActorRotation());
	}

	if (DeltaSeconds > 0.f)
	{
		// Divide turn rate by DeltaSeconds to get a consistent turning speed, scale it down (e.g., * 0.05f)
		float TargetLean = (DeltaActorYaw / DeltaSeconds) * 0.05f;
		TargetLean = FMath::Clamp(TargetLean, -90.0f, 90.0f); // Clamp to your blendspace max/min

		// Interpolate so the lean is smooth and doesn't snap
		LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 10.0f);
	}
}

void UFCFBaseAnimInstance::UpdateOrientationData()
{
	LastFrameVelocityLocomotionDirection = VelocityLocomotionDirection;
	VelocityLocomotionAngle = UKismetAnimationLibrary::CalculateDirection(CharacterVelocity2D, WorldRotation);
	VelocityLocomotionDirection = CalculateLocomotionDirection(VelocityLocomotionAngle, -50.f, 50.f, -130.f, 130.f, 20.f, VelocityLocomotionDirection);
}

void UFCFBaseAnimInstance::DrawDebug()
{
	if (DebugOptions.bShowLocomotionData)
	{
		// Velocity
		DebugPrintString("Velocity", FString::SanitizeFloat(CharacterVelocity2D.Size2D()), FLinearColor(1.0f, 0.77f, 0.0f));
		DebugDrawVector(CharacterVelocity2D, FLinearColor(1.0f, 0.77f, 0.0f), "Velocity", 150.0);

		// Acceleration
		DebugDrawVector(Acceleration2D, FLinearColor(0.0f, 0.06f, 0.59f), "Acceleration", 150.0);

		// Locomotion Angle & Direction
		DebugPrintString("VelocityLocomotionAngle", FString::SanitizeFloat(VelocityLocomotionAngle), FLinearColor(1.0f, 0.77f, 0.0f));

		FString DirString = UEnum::GetValueAsString(VelocityLocomotionDirection);
		DirString.Split(TEXT("::"), nullptr, &DirString); // Cleans up "ELocomotionDirection::Forward" to just "Forward"
		DebugPrintString("VelocityLocomotionDirection", DirString, FLinearColor(1.0f, 0.77f, 0.0f));
	}

	if (DebugOptions.bShowGaitData)
	{
		FString GaitString = UEnum::GetValueAsString(CurrentGait);
		GaitString.Split(TEXT("::"), nullptr, &GaitString);
		DebugPrintString("Current Gait", GaitString, FLinearColor(0.0f, 1.0f, 0.9f));
	}

	if (DebugOptions.bDistanceMatching && MovementComponent)
	{
		FVector StopLocation = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
			CharacterVelocity2D,
			MovementComponent->bUseSeparateBrakingFriction,
			MovementComponent->BrakingFriction,
			MovementComponent->GroundFriction,
			MovementComponent->BrakingFrictionFactor,
			MovementComponent->BrakingDecelerationWalking
		);

		FVector Center = WorldLocation + StopLocation;
		UKismetSystemLibrary::DrawDebugCapsule(this, Center, 20.0f, 20.0f, FRotator::ZeroRotator, FLinearColor(0.0f, 1.0f, 0.0f), 0.0f, 2.0f);
	}
}

void UFCFBaseAnimInstance::DebugPrintString(const FString& Key, const FString& Value, FLinearColor TextColor)
{
	if (GEngine)
	{
		FString FinalString = FString::Printf(TEXT("%s = %s"), *Key, *Value);
		// Using a key of -1 prevents it from overwriting, and a duration of 0.0 prints exactly like a Tick in BP
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, TextColor.ToFColor(true), FinalString);
	}
}

void UFCFBaseAnimInstance::DebugDrawVector(const FVector& Vector, FLinearColor LineColor, const FString& Text, double MaxLength)
{
	FVector ClampedVector = Vector.GetClampedToMaxSize(MaxLength);
	FVector LineEnd = WorldLocation + ClampedVector;

	UKismetSystemLibrary::DrawDebugArrow(this, WorldLocation, LineEnd, 5.0f, LineColor, 0.0f, 3.0f);
	UKismetSystemLibrary::DrawDebugString(this, LineEnd, Text, nullptr, LineColor, 0.0f);
}

ELocomotionDirection UFCFBaseAnimInstance::CalculateLocomotionDirection(float Angle, float ForwardMin, float ForwardMax, float BackwardMin, float BackwardMax, float Deadzone, ELocomotionDirection CurrentDir)
{
	// 1. If we are already moving Forward/Backward, we EXPAND the range (+/- Deadzone)
	if (CurrentDir == ELocomotionDirection::Forward || CurrentDir == ELocomotionDirection::Backward)
	{
		if (UKismetMathLibrary::InRange_FloatFloat(Angle, ForwardMin - Deadzone, ForwardMax + Deadzone)) return ELocomotionDirection::Forward;

		// FIX: Backward is at the edges of the -180 to 180 circle. 
		// We expand the range by checking if it's further past the thresholds (closer to 0).
		if (Angle <= (BackwardMin + Deadzone) || Angle >= (BackwardMax - Deadzone)) return ELocomotionDirection::Backward;
	}
	// 2. If we are moving Left/Right, we SHRINK the range (+/- Deadzone inverted)
	else
	{
		if (UKismetMathLibrary::InRange_FloatFloat(Angle, ForwardMin + Deadzone, ForwardMax - Deadzone)) return ELocomotionDirection::Forward;

		// FIX: Shrink the backward range (pushing the threshold closer to +/- 180)
		if (Angle <= (BackwardMin - Deadzone) || Angle >= (BackwardMax + Deadzone)) return ELocomotionDirection::Backward;
	}

	// 3. Fallback: If we aren't inside any Forward/Backward ranges, we must be strafing.
	if (Angle >= 0.f) return ELocomotionDirection::Right;

	return ELocomotionDirection::Left;
}


