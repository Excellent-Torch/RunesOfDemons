// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"



void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		TMap<ADemonCharacter*, uint32> HitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			
			ADemonCharacter* DemonCharacter = Cast<ADemonCharacter>(FireHit.GetActor());
			if (DemonCharacter)
			{
				if (HitMap.Contains(DemonCharacter))
				{
					HitMap[DemonCharacter]++;
				}
				else
				{
					HitMap.Emplace(DemonCharacter, 1);
				}
				if (SolidImpactParticles)
				{
					UNiagaraComponent* SolidImpactNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SolidImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					SolidImpactNiagaraComp->SetAutoDestroy(true);
				}
				if (SolidImpactSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						SolidImpactSound,
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-.5f, .5f)
					);
				}
			}

			
			bool bHitPlayer = false;
			ADemonCharacter* HitPlayer = Cast<ADemonCharacter>(FireHit.GetActor());
			if (HitPlayer)
			{
				bHitPlayer = true;
				HitPlayer->MulticastHit();
			}
			MulticastHitFX(FireHit.ImpactPoint, bHitPlayer);
		}
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key, // Character that was hit
					Damage * HitPair.Value, // Multiply Damage by number of times 
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			
			
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		HitTargets.Add(ToEndLoc);
	}
}

