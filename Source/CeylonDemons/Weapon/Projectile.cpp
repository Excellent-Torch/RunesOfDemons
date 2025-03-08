#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "CeylonDemons/PlayerController/DemonPlayerController.h"
#include "CeylonDemons/CeylonDemons.h"
#include "NiagaraFunctionLibrary.h"
#include <Net/UnrealNetwork.h>

AProjectile::AProjectile()
{

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;


	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

}


void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
		CollisionBox->IgnoreActorWhenMoving(Owner, true);
	}
}



void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	bGettingHit = false;
	ADemonCharacter* HitPlayer = Cast<ADemonCharacter>(OtherActor);
	if (HitPlayer && HitPlayer->IsElimmed() == false)
	{
		bGettingHit = true;
		HitPlayer->MulticastHit();
	}
	MulticastHitFX(Hit.ImpactPoint, bGettingHit);

	Destroy();
}

void AProjectile::MulticastHitFX_Implementation(FVector_NetQuantize HitLocation, bool bHitPlayer)
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
	}
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::ExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // WorldContext Object
				Damage, // Base Damage
				10.f,// Minimum Damage
				GetActorLocation(), // Origin
				DamageInnerRadius,// DamageInnerRadius
				DamageOuterRadius,//DamageOuterRadius
				1.f, // DamageFallOff
				UDamageType::StaticClass(), // DamageTypeClass
				TArray<AActor*>(), // IgnoreActors
				this,
				FiringController // InstigatorController
			);
		}
	}
}
