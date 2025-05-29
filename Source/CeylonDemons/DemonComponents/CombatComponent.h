// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CeylonDemons/HUD/DemonHUD.h"
#include "CeylonDemons/HUD/CharacterOverlay.h"
#include "CeylonDemons/Weapon/WeaponTypes.h"
#include "CeylonDemons/DemonTypes/CombatStates.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CEYLONDEMONS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatComponent();
	friend class ADemonCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapons();
	void Reload();
	void FireButtonPressed(bool bPressed);


	UFUNCTION(BlueprintCallable)
	void FinishedReloading();

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	void SetSpeeds(float InBaseSpeed, float InCrouchSpeed);

	void SetSpeedBuff(bool SpeedBuff);

	void RunButtonPressed(bool bPressed);
	bool bLocallyReloading = false;

protected:
	
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();
	

	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);


	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
	int32 AmountToReload();
	void UpdateCarriedAmmo();

	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToBack(AActor* ActorToAttach);




	void ApplyCameraShakes();


	UFUNCTION(Server, Reliable)
	void ServerRunSpeed(bool bPressed);



	
private:
	UPROPERTY()
	class ADemonCharacter* Character;
	UPROPERTY()
	class ADemonPlayerController* Controller;

	UPROPERTY()
	class UBuffComponent* Buff;


	UPROPERTY()
	class ADemonHUD* HUD;
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(Replicated)
	bool bGettingHit;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;
	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(Replicated)
	bool IsFalling;

	UPROPERTY(Replicated)
	bool bSpeedBuff;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float BaseCrouchSpeed;

	UPROPERTY(EditAnywhere)
	float BaseRunSpeed;

	UPROPERTY(EditAnywhere)
	float BuffRunSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(ReplicatedUsing = OnRep_Running)
	bool bRunning = false;
	bool bRunButtonPressed = false;

	UFUNCTION()
	void OnRep_Running();

	UPROPERTY(VisibleAnywhere)
	bool bFireButtonPressed;


	//HUD and Crosshairs

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	FHUDPackage HUDPackage;

	/**
	* Aiming and FOV
	*/

	// FOV when not aiming; set to the camera s base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float RunFOV = 120.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float RunInterpSpeed = 30.f;
	
	void InterpFOV(float DeltaTime);

	/**
	*Automatic Fire
	*/

	FTimerHandle FireTimer;
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// Carried ammo for the currently-equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	TMap<EWeaponType, int32> CarriedAmmoMap;
	
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperRifleAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState, VisibleAnywhere)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);


	
	

	/**
	* Camera Shakes
	*/

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shakes")
	TSubclassOf<UCameraShakeBase> CSM_Idle;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shakes")
	TSubclassOf<UCameraShakeBase> CSM_Aiming;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shakes")
	TSubclassOf<UCameraShakeBase> CSM_WeaponRecoil;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shakes")
	TSubclassOf<UCameraShakeBase> CSM_Run;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shakes")
	//TSubclassOf<UCameraShakeBase> CSM_Run;

	float Speed;

public:
	bool ShouldSwapWeapons();

	
};
