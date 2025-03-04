// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "CeylonDemons/DemonComponents/BuffComponent.h"



void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ADemonCharacter* DemonCharacter = Cast<ADemonCharacter>(OtherActor);

	if (DemonCharacter)
	{
		UBuffComponent* Buff = DemonCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffSpeed(true, SpeedBuffTime);
		}
	}

	Destroy();
}
