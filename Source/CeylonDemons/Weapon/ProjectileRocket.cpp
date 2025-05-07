// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "CeylonDemons/Character/DemonCharacter.h"


AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);

}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
		CollisionBox->IgnoreActorWhenMoving(Owner, true);
	}

	SpawnTrailSystem();

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
			);
	}
	
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}

	ExplodeDamage();

	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);


	StartDestroyTimer();

	if (SolidImpactParticles)
	{
		UNiagaraComponent* SolidImpactNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SolidImpactParticles, GetActorLocation());
		SolidImpactNiagaraComp->SetAutoDestroy(true);
	}
	if (SolidImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SolidImpactSound, GetActorLocation());
	}
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	/*if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		//TrailSystemComponent->GetSystemInstanceController()->Deactivate();
		
	}
	*/
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	bool bHitPlayer = false;
	ADemonCharacter* HitPlayer = Cast<ADemonCharacter>(OtherActor);
	if (HitPlayer)
	{
		bHitPlayer = true;
		HitPlayer->MulticastHit();
	}
	MulticastHitFX(Hit.ImpactPoint, Hit.ImpactNormal, bHitPlayer);

}



void AProjectileRocket::Destroyed()
{
}