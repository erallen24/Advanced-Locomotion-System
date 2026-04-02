#include "FluidityCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AFluidityCharacter::AFluidityCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFluidityCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 1. Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// 2. Link Default Animation Layer
	if (UnarmedAnimLayer && GetMesh())
	{
		GetMesh()->LinkAnimClassLayers(UnarmedAnimLayer);
	}

	// =====================================================================
	// THE FIX: Initialize the Gait system so you don't have to press Shift
	// =====================================================================
	UpdateGaitSettings(EGaitState::Run);
}

void AFluidityCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Move
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFluidityCharacter::Move);

		// Look
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFluidityCharacter::Look);

		// Switch Weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this, &AFluidityCharacter::SwitchWeapon);

		// Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AFluidityCharacter::StartAim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AFluidityCharacter::StopAim);
	}
}

void AFluidityCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// Get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFluidityCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFluidityCharacter::SwitchWeapon(const FInputActionValue& Value)
{
	// Get the truncated integer value from the Input Action
	int32 Selection = FMath::TruncToInt(Value.Get<float>());

	switch (Selection)
	{
	case 1:
		EquippedGun = EGunState::Unarmed;
		if (UnarmedAnimLayer) GetMesh()->LinkAnimClassLayers(UnarmedAnimLayer);
		break;
	case 2:
		EquippedGun = EGunState::Pistol;
		if (PistolAnimLayer) GetMesh()->LinkAnimClassLayers(PistolAnimLayer);
		break;
	case 3:
		EquippedGun = EGunState::Rifle;
		if (RifleAnimLayer) GetMesh()->LinkAnimClassLayers(RifleAnimLayer);
		break;
	default:
		break;
	}
}

void AFluidityCharacter::StartAim(const FInputActionValue& Value)
{
	UpdateGaitSettings(EGaitState::Walk);
}

void AFluidityCharacter::StopAim(const FInputActionValue& Value)
{
	UpdateGaitSettings(EGaitState::Run);
}



void AFluidityCharacter::UpdateGaitSettings(EGaitState NewGait)
{
	// 1. Set the Current Gait variable
	CurrentGait = NewGait;

	// 2. Fire the event so your Blueprint can send the interface message to the AnimInstance
	SendGaitToAnimInstance(CurrentGait);

	// 3. Find the settings for this specific gait in our Map
	if (FFluidityGaitSettings* FoundSettings = GaitSettings.Find(CurrentGait))
	{
		// 4. Apply the settings to the Character Movement Component
		if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = FoundSettings->MaxWalkSpeed;
			MovementComp->MaxAcceleration = FoundSettings->MaxAcceleration;
			MovementComp->BrakingDecelerationWalking = FoundSettings->BrakingDeceleration;
			MovementComp->BrakingFrictionFactor = FoundSettings->BrakingFrictionFactor;
			MovementComp->BrakingFriction = FoundSettings->BrakingFriction;
			MovementComp->bUseSeparateBrakingFriction = FoundSettings->bUseSeparateBrakingFriction;
		}
	}
}

void AFluidityCharacter::DebugTimeDilationSlow()
{
	UGameplayStatics::SetGlobalTimeDilation(this, 0.1f);
}

void AFluidityCharacter::DebugTimeDilationNormal()
{
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
}

