#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class CEYLONDEMONS_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void ExplodeDamage();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHitFX(FVector_NetQuantize HitLocation, FVector_NetQuantize HitRotation, bool bHitPlayer);


	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	// New Method for Character Impact Particles and Sound

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* SolidImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundBase* SolidImpactSound;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PlayerImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundBase* PlayerImpactSound;

	// ---- ///

	// Decals
	UPROPERTY(EditDefaultsOnly, Category = "Decals")
	UMaterialInterface* ImpactDecal;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

private:

	UPROPERTY()
	class ADemonCharacter* DemonOwnerCharacter;

	UPROPERTY()
	class ADemonPlayerController* DemonOwnerController;


	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	/*
	* Different Hit Particles
	**/
	
	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;


	UPROPERTY(EditAnywhere)
	bool bGettingHit;
public:	

};
