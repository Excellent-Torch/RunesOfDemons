
#include "BulletShell.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"


ABulletShell::ABulletShell()
{
	PrimaryActorTick.bCanEverTick = false;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	SetRootComponent(BulletMesh);
	BulletMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	BulletMesh->SetSimulatePhysics(true);
	BulletMesh->SetEnableGravity(true);
	BulletMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
}


void ABulletShell::BeginPlay()
{
	Super::BeginPlay();

	FVector RandomShell = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(GetActorForwardVector(), 20.f);

	BulletMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	BulletMesh->AddImpulse(RandomShell * ShellEjectionImpulse);
		
}

void ABulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound )
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	Destroy();
}



