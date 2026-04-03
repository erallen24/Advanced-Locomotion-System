// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"

#include "FCFBaseAnimInstance.h" // We need this to read your Base variables!
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"

#include "FCFLayersAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FFluidityDirectionalAnimations
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimSequence* Forward = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimSequence* Backward = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimSequence* Left = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimSequence* Right = nullptr;
};

/**
 *
 */
UCLASS()
class THIRDPERSON_ALS_API UFCFLayersAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	// Automatically grabs and caches your ABP_Base!
	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|References")
	UFCFBaseAnimInstance* MainAnimInstance;

protected:

	// Your Animation Setup Variables (Fill these out in the Blueprint!)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Animations|Cycle")
	FFluidityDirectionalAnimations WalkCycleAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Animations|Cycle")
	FFluidityDirectionalAnimations JogCycleAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Animations|Stop")
	FFluidityDirectionalAnimations WalkStopAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Animations|Stop")
	FFluidityDirectionalAnimations JogStopAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Idle")
	class UAnimSequence* IdleAnimation;

public:
	// =======================================================================
	// Anim Node Functions (Thread-Safe replacements for your BP functions)
	// =======================================================================

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void CycleOnUpdate(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupStopAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateStopAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void IdleOnUpdate(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

private:
	// Helper function to pick the right animation based on the current direction
	UAnimSequence* GetAnimFromDirection(const FFluidityDirectionalAnimations& AnimStruct, ELocomotionDirection Direction);
};
