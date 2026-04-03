/// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/FCFLayersAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SequencePlayerLibrary.h"
#include "SequenceEvaluatorLibrary.h"
#include "AnimDistanceMatchingLibrary.h"
#include "AnimCharacterMovementLibrary.h"

void UFCFLayersAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Looks at the Component playing this layer, and gets its main AnimInstance.
	if (USkeletalMeshComponent* MeshComp = GetOwningComponent())
	{
		MainAnimInstance = Cast<UFCFBaseAnimInstance>(MeshComp->GetAnimInstance());
	}
}

void UFCFLayersAnimInstance::CycleOnUpdate(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MainAnimInstance) return;

	// 1. Get the required data from the Base Anim Instance
	EGaitState CurrentGait = MainAnimInstance->CurrentGait;
	ELocomotionDirection CurrentDir = MainAnimInstance->VelocityLocomotionDirection;

	// 2. Figure out which animation we should be playing
	UAnimSequence* SelectedSequence = nullptr;

	if (CurrentGait == EGaitState::Walk)
	{
		SelectedSequence = GetAnimFromDirection(WalkCycleAnimations, CurrentDir);
	}
	else
	{
		SelectedSequence = GetAnimFromDirection(JogCycleAnimations, CurrentDir);
	}

	// 3. Apply it to the Sequence Player Node with Inertial Blending
	EAnimNodeReferenceConversionResult ConversionResult;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, ConversionResult);

	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, SelectedSequence, 0.2f);
	}
}

void UFCFLayersAnimInstance::SetupStopAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MainAnimInstance) return;

	EGaitState CurrentGait = MainAnimInstance->CurrentGait;
	ELocomotionDirection CurrentDir = MainAnimInstance->VelocityLocomotionDirection;

	UAnimSequence* SelectedSequence = nullptr;

	if (CurrentGait == EGaitState::Walk)
	{
		SelectedSequence = GetAnimFromDirection(WalkStopAnimations, CurrentDir);
	}
	else
	{
		SelectedSequence = GetAnimFromDirection(JogStopAnimations, CurrentDir);
	}

	EAnimNodeReferenceConversionResult ConversionResult;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);

	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, SelectedSequence);
	}
}

void UFCFLayersAnimInstance::UpdateStopAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MainAnimInstance || !MainAnimInstance->MovementComponent) return;

	EAnimNodeReferenceConversionResult ConversionResult;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);

	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		// Advance the time normally if we are still accelerating
		if (MainAnimInstance->IsAccelerating)
		{
			USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEvaluator, 1.0f);
		}
		// If we are stopping, use Distance Matching to scrub the animation!
		else
		{
			FVector Velocity = MainAnimInstance->MovementComponent->Velocity;
			bool bUseSeparateBraking = MainAnimInstance->MovementComponent->bUseSeparateBrakingFriction;
			float BrakingFriction = MainAnimInstance->MovementComponent->BrakingFriction;
			float GroundFriction = MainAnimInstance->MovementComponent->GroundFriction;
			float FrictionFactor = MainAnimInstance->MovementComponent->BrakingFrictionFactor;
			float BrakingDecel = MainAnimInstance->MovementComponent->BrakingDecelerationWalking;

			// Predict how far it will take us to completely stop sliding
			FVector StopLocation = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
				Velocity, bUseSeparateBraking, BrakingFriction, GroundFriction, FrictionFactor, BrakingDecel);

			float StopDistance = StopLocation.Size2D();

			// Scrub the animation to perfectly match our stopping distance
			UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, StopDistance, FName("Distance"));
		}
	}
}

void UFCFLayersAnimInstance::IdleOnUpdate(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	// Ensure we have a valid animation to play
	if (!IdleAnimation) return;

	EAnimNodeReferenceConversionResult ConversionResult;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, ConversionResult);

	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		// Set the sequence on the Sequence Player node
		USequencePlayerLibrary::SetSequence(SequencePlayer, IdleAnimation);
	}
}

// Simple helper to clean up our code
UAnimSequence* UFCFLayersAnimInstance::GetAnimFromDirection(const FFluidityDirectionalAnimations& AnimStruct, ELocomotionDirection Direction)
{
	switch (Direction)
	{
	case ELocomotionDirection::Forward: return AnimStruct.Forward;
	case ELocomotionDirection::Backward: return AnimStruct.Backward;
	case ELocomotionDirection::Right: return AnimStruct.Right;
	case ELocomotionDirection::Left: return AnimStruct.Left;
	default: return AnimStruct.Forward;
	}
}