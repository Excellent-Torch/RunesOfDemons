// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class CEYLONDEMONS_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
protected:
	
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHitFX(FVector_NetQuantize HitLocation, FVector_NetQuantize HitRotation, bool bHitPlayer);

	// New Method for Character Impact Particles and Sound

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* SolidImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* SolidImpactSound;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PlayerImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* PlayerImpactSound;

	// ---- ///

	// Decals
	UPROPERTY(EditDefaultsOnly, Category = "Decals")
	UMaterialInterface* ImpactDecal;

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

private:

	

};
