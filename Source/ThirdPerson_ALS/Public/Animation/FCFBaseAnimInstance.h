// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FluidityCharacter.h"
#include "FCFBaseAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ELocomotionDirection : uint8
{
	Forward		UMETA(DisplayName = "Forward"),
	Backward	UMETA(DisplayName = "Backward"),
	Left		UMETA(DisplayName = "Left"),
	Right		UMETA(DisplayName = "Right")
};

/**
 *
 */
UCLASS()
class THIRDPERSON_ALS_API UFCFBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	// This matches your "Blueprint Thread Safe Update Animation" node!
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|State")
	EGaitState CurrentGait;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	ELocomotionDirection VelocityLocomotionDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	ELocomotionDirection LastFrameVelocityLocomotionDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	bool IsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|References")
	class UCharacterMovementComponent* MovementComponent;

protected:
	/** Essential References */
	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|References")
	class AFluidityCharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|State")
	EGaitState LastFrameGait;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|State")
	bool IsGaitChanged;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|State")
	EGunState EquippedGun;

	/** Locomotion Data */
	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	FVector CharacterVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	FVector CharacterVelocity2D;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	FVector Acceleration;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	FVector Acceleration2D;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	float VelocityLocomotionAngle;

	

	/** Rotation & Location Data */
	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	FVector WorldLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	FRotator WorldRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	float ActorYaw;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	float LastFrameActorYaw;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	float DeltaActorYaw;

	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	float LeanAngle;

protected:
	/** Modular Update Functions (Replicating your BP structure) */
	UFUNCTION(BlueprintCallable, Category = "Fluidity|Update")
	void GetCharacterStates();

	UFUNCTION(BlueprintCallable, Category = "Fluidity|Update")
	void GetVelocityData();

	UFUNCTION(BlueprintCallable, Category = "Fluidity|Update")
	void GetAccelerationData();

	UFUNCTION(BlueprintCallable, Category = "Fluidity|Update")
	void GetLocationData();

	UFUNCTION(BlueprintCallable, Category = "Fluidity|Update")
	void GetRotationData(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "Fluidity|Update")
	void UpdateOrientationData();

private:
	ELocomotionDirection CalculateLocomotionDirection(float Angle, float ForwardMin, float ForwardMax, float BackwardMin, float BackwardMax, float Deadzone, ELocomotionDirection CurrentDir);
};
