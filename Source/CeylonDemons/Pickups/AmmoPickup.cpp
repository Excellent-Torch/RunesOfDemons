// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "CeylonDemons/DemonComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ADemonCharacter* DemonCharacter = Cast<ADemonCharacter>(OtherActor);

	if (DemonCharacter)
	{
		UCombatComponent* Combat = DemonCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}

	Destroy();
}
