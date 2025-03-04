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
	void MulticastHitFX(FVector_NetQuantize HitLocation, bool bHitPlayer);

	UPROPERTY(EditAnywhere)
	float Damage = 25.f;

	// New Method for Character Impact Particles and Sound

	UPROPERTY(EditAnywhere)
	class UParticleSystem* SolidImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* SolidImpactSound;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* PlayerImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* PlayerImpactSound;

	// ---- ///

	

private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

};
