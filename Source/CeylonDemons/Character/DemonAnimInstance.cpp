// Fill out your copyright notice in the Description page of Project Settings.


#include "DemonAnimInstance.h"
#include "DemonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "CeylonDemons/Weapon/Weapon.h"
#include "CeylonDemons/DemonTypes/CombatStates.h"

void UDemonAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	DemonCharacter = Cast<ADemonCharacter>(TryGetPawnOwner());
}

void UDemonAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (DemonCharacter == nullptr)
	{
		DemonCharacter = Cast<ADemonCharacter>(TryGetPawnOwner());
		
	}
	if (DemonCharacter == nullptr) return;

	FVector Velocity = DemonCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();


	//FString SSpeed = FString::Printf(TEXT("%f"),Speed);

	//if (GEngine)
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, SSpeed);

	bIsInAir = DemonCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = DemonCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = DemonCharacter->IsWeaponEquipped();
	EquippedWeapon = DemonCharacter->GetEquippedWeapon();
	bIsCrouched = DemonCharacter->bIsCrouched;
	bAiming = DemonCharacter->IsAiming();
	bRunning = DemonCharacter->IsRunning();
	TurningInPlace = DemonCharacter->GetTurningInPlace();
	bRotateRootBone = DemonCharacter->ShouldRotateRootBone();
	bElimmed = DemonCharacter->IsElimmed();

	// Offset Yaw for Strafing
	FRotator AimRotation = DemonCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(DemonCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = DemonCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = DemonCharacter->GetAO_Yaw();
	AO_Pitch = DemonCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && DemonCharacter->GetMesh() && !DemonCharacter->IsRunning())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		DemonCharacter->GetMesh()->TransformToBoneSpace(FName("J_Bip_R_Hand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));


		/*Debug Lines*/


		//FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), DemonCharacter->GetHitTarget(), FColor::Orange);

	}


	bUseFABRIK = DemonCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;

	if (DemonCharacter->IsLocallyControlled() && DemonCharacter->GetCombatState() != ECombatState::ECS_MAX)
	{
		bUseFABRIK = !DemonCharacter->IsLocallyReloading() && !DemonCharacter->IsRunning();
	}

	bUseAimOffsets = DemonCharacter->GetCombatState() == ECombatState::ECS_Reloading && !DemonCharacter->GetDisableGameplay();
	bTransformRightHand = DemonCharacter->GetCombatState() == ECombatState::ECS_Reloading && !DemonCharacter->GetDisableGameplay();
}

