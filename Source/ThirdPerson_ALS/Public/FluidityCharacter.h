#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "FluidityCharacter.generated.h"

UENUM(BlueprintType)
enum class EGunState : uint8
{
	Unarmed		UMETA(DisplayName = "Unarmed"),
	Pistol		UMETA(DisplayName = "Pistol"),
	Rifle		UMETA(DisplayName = "Rifle")
};

UENUM(BlueprintType)
enum class EGaitState : uint8
{
	Walk		UMETA(DisplayName = "Walk"),
	Run			UMETA(DisplayName = "Run")
};

USTRUCT(BlueprintType)
struct FFluidityGaitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxWalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAcceleration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingDeceleration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingFrictionFactor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingFriction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseSeparateBrakingFriction = false;
};

UCLASS()
class THIRDPERSON_ALS_API AFluidityCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFluidityCharacter();

	// =================================================================
	// MOVED TO PUBLIC: Now the AnimInstance can safely read these!
	// =================================================================

	// The current active gait
	UPROPERTY(BlueprintReadOnly, Category = "Fluidity|Locomotion")
	EGaitState CurrentGait = EGaitState::Run;

	// State Variables
	UPROPERTY(BlueprintReadWrite, Category = "Fluidity|State")
	EGunState EquippedGun = EGunState::Unarmed;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Locomotion")
	TMap<EGaitState, FFluidityGaitSettings> GaitSettings;

	UFUNCTION(BlueprintImplementableEvent, Category = "Fluidity|Locomotion")
	void SendGaitToAnimInstance(EGaitState NewGait);

	/* * Enhanced Input
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Input")
	class UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Input")
	class UInputAction* SwitchWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Input")
	class UInputAction* AimAction;

	/* * Animation Layers
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Animation")
	TSubclassOf<UAnimInstance> UnarmedAnimLayer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Animation")
	TSubclassOf<UAnimInstance> PistolAnimLayer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fluidity|Animation")
	TSubclassOf<UAnimInstance> RifleAnimLayer;

	/* * Input Callbacks
	 */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SwitchWeapon(const FInputActionValue& Value);
	void StartAim(const FInputActionValue& Value);
	void StopAim(const FInputActionValue& Value);

	// Debug Inputs
	void DebugTimeDilationSlow();
	void DebugTimeDilationNormal();

	/*
	 * Custom Functions
	 */
	UFUNCTION(BlueprintCallable, Category = "Fluidity|Locomotion")
	void UpdateGaitSettings(EGaitState NewGait);
};
