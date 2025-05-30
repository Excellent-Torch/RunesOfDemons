// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CeylonDemons/DemonTypes/TurningInPlace.h"
#include "CeylonDemons/Interface/InteractWithCrosshairsInterface.h"
#include "CeylonDemons/DemonTypes/CombatStates.h"
#include "InputActionValue.h"
#include "MediaPlayer.h"
#include "DemonCharacter.generated.h"


USTRUCT(BlueprintType)
struct FPhysicsAssetInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FName BoneName;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	float HalfHeight;

	UPROPERTY()
	float Radius;

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);


class UInputMappingContext;
class UInputComponent;
class UInputAction;
class UInputMappingContext;
class UCapsuleComponent;
class UBoxComponent;


struct FInputActionValue;



UCLASS()
class CEYLONDEMONS_API ADemonCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ADemonCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	
	// PLAYER INPUTS

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* IA_Look;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* IA_Jump;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* IA_Run;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* IA_Fire;


	// END PLAYER INPUTS


	UFUNCTION(NetMulticast, Reliable)
	void MulticastHit();

	virtual void OnRep_ReplicatedMovement() override;
	void Elim(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	//void BeginPlay() override;

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDAmmo();
	void SpawnDefaultWeapon();

	// Experimental 3D Health Bar
	
	UFUNCTION()
	void SetMeshHealthBar();

	UFUNCTION()
	void UpdateMeshHealthBar(float HealthMeshStat);

	UPROPERTY(VisibleAnywhere)
	UMaterialInterface* HealthMaterial;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* HealthDynamicMaterialInstance;

	// Experimental 3D Shield Bar

	UFUNCTION()
	void SetMeshShieldBar();

	UFUNCTION()
	void UpdateMeshShieldBar(float ShieldMeshStat);

	UPROPERTY(VisibleAnywhere)
	UMaterialInterface* ShieldMaterial;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* ShieldDynamicMaterialInstance;

	// Kill Z

	void SavePlayerLocation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FVector SafeLocation;

	bool bIsFalling;
	FTimerHandle SaveTimerHandle;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	// Hit Boxes
	UPROPERTY()
	TMap<FName, class UCapsuleComponent*> HitCapsuleCollisions;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//TArray<FPhysicsAssetInformation> PhysicsAssetInfo;

	UFUNCTION()
	void SetupHitBox();

	UFUNCTION()
	void ConstructHitBox(FPhysicsAssetInformation& PhysicsAssetInfo);

	UFUNCTION()
	void OnHeadBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(Server, Reliable)
	void ServerHeadBoxHit(bool bHeadHit);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHeadBoxHit(bool bHeadHit);


	UPROPERTY(Replicated)
	bool bHeadShot = false;

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	virtual void Jump() override;
	void Run();
	void StopRun();
	void SwapButtonPressed();
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();

	
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	

	// Poll for any relevant classes and initialize our HUD
	void PollInit();
	void RotateInPlace(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	//UPROPERTY(EditAnywhere)
	//class UStaticMesh* HealthBarMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* HealthBarMesh;

	//UPROPERTY(EditAnywhere)
	//class UStaticMesh* ShieldBarMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ShieldBarMesh;

	UPROPERTY(VisibleAnywhere, Category = "HitBox")
	class UBoxComponent* HeadBox;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere)
	class ULagComponent* LagComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UDestructionComponent* DestructionComponent;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerSwapButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	//UPROPERTY(EditAnywhere, Category = Combat)
	//class UAnimMontage* FireWeaponMontage;

	//UPROPERTY(EditAnywhere, Category = Combat)
	//UAnimMontage* HitReactMontage;

	//UPROPERTY(EditAnywhere, Category = Combat)
	//UAnimMontage* ElimMontage;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	
	/**
	* Player Health
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Player Stats")
	float MaxMeshHealth = 0.5f;

	UPROPERTY(VisibleAnywhere, Category = "Player Stats")
	float MeshHealth = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UPROPERTY()
	class ADemonPlayerController* DemonPlayerController;

	/**
	* Player shield
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Player Stats")
	float MaxMeshShield = 0.5f;

	UPROPERTY(VisibleAnywhere, Category = "Player Stats")
	float MeshShield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	/**
	* Player Elimination
	*/
	

	bool bGettingHit = false;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	
	void ElimTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 0.f;


	/**
	* Elim Cyclone
	*/

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimCycloneEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimCycloneComponent;

	UPROPERTY(ReplicatedUsing = OnRep_DamageScore, VisibleAnywhere, Category = "Damage Stats")
	int32 DamageScore = 100.f;

	UFUNCTION()
	void OnRep_DamageScore(int32 DmgScore);

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimSound;

	UPROPERTY(EditAnywhere)
	class USoundCue* HeadShotElimSound;

	UPROPERTY()
	class ADemonPlayerState* DemonPlayerState;


	UPROPERTY(VisibleAnywhere, Category = "Physics")
	float ElimImpulseStrength = 10.0f;

	/**
	*	Lead Player Effects
	*/

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem;

	UPROPERTY()
	class UNiagaraComponent* CrownComponent;

	/**
	* Default weapon
	*/

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	bool bLeftGame = false;


	// Running
	bool bLocallyRunning = false;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();
	bool IsAiming();
	bool IsRunning();
	bool IsLocallyReloading();

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	UFUNCTION(BlueprintCallable)
	bool IsElimmed() const { return bElimmed; }

	FORCEINLINE bool IsGettingHit() const { return bGettingHit; }

	UFUNCTION(BlueprintCallable)
	bool IsHeadShot() const { return bHeadShot; }
	
	UFUNCTION(BlueprintCallable)
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable)
	float GetShield() const { return Shield; }

	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	//FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	FORCEINLINE int32 GetDamageScore() const { return DamageScore; }
	FORCEINLINE ULagComponent* GetLagComponent() const { return LagComponent; }
	FORCEINLINE UDestructionComponent* GetDestructionComponent() const { return DestructionComponent; }

	// Animation Montages

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	UAnimMontage* ElimMontage;

	// Special Effects

	void UpdateSpecialEffects();

};

