// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "CeylonDemons/PlayerController/DemonPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"
#include "DrawDebugHelpers.h"
#include <Net/UnrealNetwork.h>
#include "CeylonDemons/DemonComponents/LagComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ADemonCharacter* DemonCharacter = Cast<ADemonCharacter>(FireHit.GetActor());
		if (DemonCharacter && InstigatorController)
		{
			if (HasAuthority() && !bUseServerSideRewind && !FireHit.GetComponent()->GetName().Contains("HeadBox"))
			{
				MulticastHitFX(FireHit.ImpactPoint, FireHit.ImpactNormal, true);

				DemonCharacter->ServerHeadBoxHit(false);
				UGameplayStatics::ApplyDamage(
					DemonCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			if (HasAuthority() && !bUseServerSideRewind && FireHit.GetComponent()->GetName().Contains("HeadBox"))
			{
				MulticastHitFX(FireHit.ImpactPoint, FireHit.ImpactNormal, true);

				DemonCharacter->ServerHeadBoxHit(true);

				UGameplayStatics::ApplyDamage(
					DemonCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}


			if (!HasAuthority() && bUseServerSideRewind)
			{
				DemonOwnerCharacter = DemonOwnerCharacter == nullptr ? Cast<ADemonCharacter>(OwnerPawn) : DemonOwnerCharacter;
				DemonOwnerController = DemonOwnerController == nullptr ? Cast<ADemonPlayerController>(InstigatorController) : DemonOwnerController;

				if (DemonOwnerController && DemonOwnerCharacter && DemonOwnerCharacter->GetLagComponent())
				{
					DemonOwnerCharacter->GetLagComponent()->ServerScoreRequest(
						DemonCharacter,
						Start,
						HitTarget,
						DemonOwnerController->GetServerTime() - DemonOwnerController->SingleTripTime,
						this
					);
				}
			}
			
		}
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
		
		if (DemonCharacter == nullptr)
		{
			MulticastHitFX(FireHit.ImpactPoint, FireHit.ImpactNormal, false);
		}
	}
}

void AHitScanWeapon::MulticastHitFX_Implementation(FVector_NetQuantize HitLocation, FVector_NetQuantize HitRotation, bool bHitPlayer)
{
	if (bHitPlayer)
	{
		if (PlayerImpactParticles)
		{
			UNiagaraComponent* PlayerImpactNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PlayerImpactParticles, HitLocation);
			PlayerImpactNiagaraComp->SetAutoDestroy(true);			
		}
		if (PlayerImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PlayerImpactSound, HitLocation);
		}
	}
	else
	{
		if (SolidImpactParticles)
		{
			UNiagaraComponent* SolidImpactNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SolidImpactParticles, HitLocation);
			SolidImpactNiagaraComp->SetAutoDestroy(true);
		}
		if (SolidImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SolidImpactSound, HitLocation);
		}

		if (ImpactDecal)
		{

			FRotator DecalRotation = HitRotation.Rotation();

			DecalRotation.Pitch -= 0.f;

			FVector DecalSize = FVector(10, 10, 10);

			UGameplayStatics::SpawnDecalAtLocation(
				GetWorld(),
				ImpactDecal,
				DecalSize,
				HitLocation,
				DecalRotation,
				10.0f // Lifespan in seconds
			);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	FHitResult FireHit;
	UWorld* World = GetWorld();
	if (World)
	{

		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		//FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		// DEBUG SPHERE !!
		//DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}







