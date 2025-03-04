// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"
#include "DrawDebugHelpers.h"
#include <Net/UnrealNetwork.h>

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
		if (DemonCharacter && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(
				DemonCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
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

		bool bHitPlayer = false;
		ADemonCharacter* HitPlayer = Cast<ADemonCharacter>(FireHit.GetActor());
		if (HitPlayer && HitPlayer->IsElimmed() == false)
		{
			bHitPlayer = true;
			HitPlayer->MulticastHit();
		}
		MulticastHitFX(FireHit.ImpactPoint, bHitPlayer);
	}
}

void AHitScanWeapon::MulticastHitFX_Implementation(FVector_NetQuantize HitLocation, bool bHitPlayer)
{
	if (bHitPlayer)
	{
		if (PlayerImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerImpactParticles, HitLocation);
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
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SolidImpactParticles, HitLocation);
		}
		if (SolidImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SolidImpactSound, HitLocation);
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







