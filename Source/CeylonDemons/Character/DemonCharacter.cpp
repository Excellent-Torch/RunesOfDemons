// Fill out your copyright notice in the Description page of Project Settings.


#include "DemonCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"

#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"


#include "Net/UnrealNetwork.h"
#include "CeylonDemons/Weapon/Weapon.h"
#include "CeylonDemons/DemonComponents/CombatComponent.h"
#include "CeylonDemons/DemonComponents/BuffComponent.h"
#include "CeylonDemons/DemonComponents/LagComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CeylonDemons/Character/DemonAnimInstance.h"
#include "CeylonDemons/CeylonDemons.h"
#include "CeylonDemons/PlayerController/DemonPlayerController.h"
#include "CeylonDemons/GameMode/DemonGameMode.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"
#include "CeylonDemons/PlayerState/DemonPlayerState.h"
#include "CeylonDemons/Weapon/WeaponTypes.h"
#include "CeylonDemons/HUD/Announcement.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "CeylonDemons/GameState/DemonGameState.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"




ADemonCharacter::ADemonCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("Buff Component"));
	Buff->SetIsReplicated(true);

	LagComponent = CreateDefaultSubobject<ULagComponent>(TEXT("LagComponent"));

	HealthBarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HealthBar Component"));
	//HealthBarMesh->SetStaticMesh(HealthBarMeshComponent);
	HealthBarMesh->SetupAttachment(RootComponent);
	HealthBarMesh->SetIsReplicated(false);

	ShieldBarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldBar Component"));
	//ShieldBarMesh->SetStaticMesh(ShieldBarMeshComponent);
	ShieldBarMesh->SetupAttachment(RootComponent);
	ShieldBarMesh->SetIsReplicated(false);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);

	// Head Box
	HeadBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadBox Component"));
	HeadBox->SetupAttachment(GetMesh(), FName("J_Bip_C_Head"));
	
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	HeadBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Block);

	HeadBox->OnComponentHit.AddDynamic(this, &ADemonCharacter::OnHeadBoxHit);

	// END Head Box

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);
}

void ADemonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADemonCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ADemonCharacter, Health);
	DOREPLIFETIME(ADemonCharacter, Shield);
	DOREPLIFETIME(ADemonCharacter, DamageScore);
	DOREPLIFETIME(ADemonCharacter, bDisableGameplay);
	DOREPLIFETIME(ADemonCharacter, bHeadShot);
}

void ADemonCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
}

void ADemonCharacter::Elim(bool bPlayerLeftGame)
{
	MulticastElim(bPlayerLeftGame);
	DropOrDestroyWeapons();
}

void ADemonCharacter::ElimTimerFinished()
{
	ADemonGameMode* DemonGameMode = GetWorld()->GetAuthGameMode<ADemonGameMode>();
	if (DemonGameMode && !bLeftGame)
	{
		DemonGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ADemonCharacter::OnRep_DamageScore(int32 DmgScore)
{
	if (bHeadShot)
	{
		DamageScore = DmgScore + 50;
	}
	else
	{
		DamageScore = DmgScore;
	}
}
	

void ADemonCharacter::ServerLeaveGame_Implementation()
{
	ADemonGameMode* DemonGameMode = GetWorld()->GetAuthGameMode<ADemonGameMode>();
	DemonPlayerState = DemonPlayerState == nullptr ? GetPlayerState<ADemonPlayerState>() : DemonPlayerState;
	if (DemonGameMode && DemonPlayerState)
	{
		DemonGameMode->PlayerLeftGame(DemonPlayerState);
	}
}

void ADemonCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	if (DemonPlayerController)
	{
		DemonPlayerController->SetHUDWeaponAmmo(0);
	}

	
	
	
	bElimmed = true;

	if (bHeadShot && ElimSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			HeadShotElimSound,
			GetActorLocation()
		);

		// Spawn Elim Cyclone
		if (ElimCycloneEffect)
		{
			FVector ElimCycloneSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
			ElimCycloneComponent = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ElimCycloneEffect,
				ElimCycloneSpawnPoint,
				GetActorRotation()
			);
		}
	}
	else
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimSound,
			GetActorLocation()
		);
	}


	// Disable Character Movement
	//GetCharacterMovement()->DisableMovement();
	//GetCharacterMovement()->StopMovementImmediately();

	if (Combat && Combat->EquippedWeapon)
	{
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			ElimImpulseStrength = 10.f;
			break;
		case EWeaponType::EWT_RocketLauncher:
			ElimImpulseStrength = 500.f;
			break;
		case EWeaponType::EWT_Pistol:
			ElimImpulseStrength = 10.f;
			break;
		case EWeaponType::EWT_SubmachineGun:
			ElimImpulseStrength = 8.f;
			break;
		case EWeaponType::EWT_Shotgun:
			ElimImpulseStrength = 50.f;
			break;
		case EWeaponType::EWT_SniperRifle:
			ElimImpulseStrength = 5.f;
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			ElimImpulseStrength = 50.f;
			break;
		}
		// Enable physics simulation on the mesh
		USkeletalMeshComponent* MeshComp = GetMesh();
		if (MeshComp)
		{
			MeshComp->SetSimulatePhysics(true);
			//MeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

			// Apply an impulse
			FVector ImpulseDirection = FVector(FMath::RandRange(-200.0f, 200.0f), FMath::RandRange(-200.0f, 200.0f), 500.0f);

			//FName HeadBone = "J_Bip_C_Head";
			//if (bHeadShot) MeshComp->BreakConstraint(FVector::ZeroVector, FVector::ZeroVector, HeadBone);

			MeshComp->AddImpulse(ImpulseDirection * ElimImpulseStrength, NAME_None, true);
		}
	}



	bDisableGameplay = true;

	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;

	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ADemonCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void ADemonCharacter::SavePlayerLocation()
{
	if (this->GetCharacterMovement()->IsFalling())
	{
		bIsFalling = true;
	}
	else 
	{
		SafeLocation = this->GetActorLocation();
	}
}

void ADemonCharacter::SetupHitBox()
{
	if (GetMesh() == nullptr) return;
	UPhysicsAsset* PhysicsAsset = GetMesh()->GetPhysicsAsset();
	if (!PhysicsAsset) return;

	FPhysicsAssetInformation CapsuleInformation;

	/*
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("ASSSSS!"));
	*/

	for (auto& BodySetup : PhysicsAsset->SkeletalBodySetups)
	{
		if (BodySetup) continue;

		FName BoneName = BodySetup->BoneName;
		int32 BoneIndex = GetMesh()->GetBoneIndex(BoneName);
		if (BoneIndex == INDEX_NONE) continue;

		FTransform BoneTransform = GetMesh()->GetBoneTransform(BoneIndex);

		
		for (auto CapsuleElem : BodySetup->AggGeom.SphylElems)
		{
			//BoneTransform.TransformPosition(CapsuleElem.Center);
			CapsuleInformation.BoneName = BoneName;
			CapsuleInformation.Transform = CapsuleElem.GetTransform() * BoneTransform;
			CapsuleInformation.Rotation = CapsuleElem.Rotation;
			CapsuleInformation.HalfHeight = CapsuleElem.Length * 0.5f;
			CapsuleInformation.Radius = CapsuleElem.Radius;

			//PhysicsAssetInfo.Add(CapsuleInformation);

			ConstructHitBox(CapsuleInformation);
			//CapsuleInformation.Add(BoneName, CapsuleInformation);
		}
	}
}

void ADemonCharacter::ConstructHitBox(FPhysicsAssetInformation& PhysicsAssetInfo)
{
	UCapsuleComponent* HitCapsule;
	HitCapsule = NewObject<UCapsuleComponent>(this, UCapsuleComponent::StaticClass(), PhysicsAssetInfo.BoneName, RF_Transient);

	HitCapsule->SetupAttachment(GetMesh(), PhysicsAssetInfo.BoneName);
	HitCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCapsule->SetWorldTransform(PhysicsAssetInfo.Transform);
	HitCapsule->SetCapsuleHalfHeight(PhysicsAssetInfo.HalfHeight);
	HitCapsule->SetCapsuleRadius(PhysicsAssetInfo.Radius);

	HitCapsule->RegisterComponent();
	if (HitCapsule)
	{
		HitCapsuleCollisions.Add(PhysicsAssetInfo.BoneName, HitCapsule);

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, PhysicsAssetInfo.BoneName.ToString());
	}
}

void ADemonCharacter::OnHeadBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//if (!HasAuthority()) return;

	if (OtherActor && OtherActor != this)
	{
		ServerHeadBoxHit(true);
	}
	else
	{
		ServerHeadBoxHit(false);
	}
}

void ADemonCharacter::ServerHeadBoxHit_Implementation(bool bHeadHit)
{
	bHeadShot = bHeadHit;
	MulticastHeadBoxHit(bHeadHit);

}

void ADemonCharacter::MulticastHeadBoxHit_Implementation(bool bHeadHit)
{
	bHeadShot = bHeadHit;
}

void ADemonCharacter::BeginPlay()
{
	Super::BeginPlay();

	//SetupHitBox();
	UpdateHUDAmmo();
	SetMeshHealthBar();
	SetMeshShieldBar();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ADemonCharacter::ReceiveDamage);
	}
}

void ADemonCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimCycloneComponent)
	{
		ElimCycloneComponent->DestroyComponent();
	}

	ADemonGameMode* DemonGameMode = Cast<ADemonGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = DemonGameMode && DemonGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}

}

void ADemonCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ADemonCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}


void ADemonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();


}

void ADemonCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ADemonCharacter::UpdateHUDAmmo()
{
	DemonPlayerController = DemonPlayerController == nullptr ? Cast<ADemonPlayerController>(Controller) : DemonPlayerController;
	if (DemonPlayerController && Combat && Combat->EquippedWeapon)
	{
		DemonPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		DemonPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ADemonCharacter::SpawnDefaultWeapon()
{
	ADemonGameMode* DemonGameMode = Cast<ADemonGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (DemonGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}




void ADemonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ADemonCharacter::Move);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ADemonCharacter::Look);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ADemonCharacter::Jump);
		EnhancedInputComponent->BindAction(IA_Run, ETriggerEvent::Triggered, this, &ADemonCharacter::Run);
		EnhancedInputComponent->BindAction(IA_Run, ETriggerEvent::Completed, this, &ADemonCharacter::StopRun);
	}

	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ADemonCharacter::Jump);
	//PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ADemonCharacter::Run);
	//PlayerInputComponent->BindAction("Run", IE_Released, this, &ADemonCharacter::StopRun);

	//PlayerInputComponent->BindAxis("MoveForward", this, &ADemonCharacter::MoveForward);
	//PlayerInputComponent->BindAxis("MoveRight", this, &ADemonCharacter::MoveRight);
	//PlayerInputComponent->BindAxis("Turn", this, &ADemonCharacter::Turn);
	//PlayerInputComponent->BindAxis("LookUp", this, &ADemonCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ADemonCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Swap", IE_Pressed, this, &ADemonCharacter::SwapButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ADemonCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ADemonCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ADemonCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ADemonCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ADemonCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ADemonCharacter::ReloadButtonPressed);
}

void ADemonCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed,
			GetCharacterMovement()->MaxWalkSpeedCrouched
		);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagComponent)
	{
		LagComponent->Character = this;
		if (Controller)
		{
			LagComponent->Controller = Cast<ADemonPlayerController>(Controller);
		}
	}
}

void ADemonCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ADemonCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		
		

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ADemonCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		
		AnimInstance->Montage_Play(ElimMontage);
		
	}
}

void ADemonCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ADemonCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}	
}

void ADemonCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}

void ADemonCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (bElimmed) return;

	float DamageToHealth = Damage;

	if (bHeadShot)
	{
		DamageToHealth = Damage * 2.f;
	}

	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			Shield = 0.f;
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

	MeshShield = FMath::Clamp(Shield * -1 / 100 + 0.5, -0.5f, MaxMeshShield);
	MeshHealth = FMath::Clamp(Health * -1 / 100 + 0.5, -0.5f, MaxMeshHealth);

	UpdateMeshHealthBar(MeshHealth);
	UpdateMeshShieldBar(MeshShield);

	PlayHitReactMontage();


	ADemonPlayerController* AttackerController = Cast<ADemonPlayerController>(InstigatorController);
	if (AttackerController)
	{
		AttackerController->SetHUDHitMark(true);


		
	}

	if (Health == 0.f)
	{
		ADemonGameMode* DemonGameMode = GetWorld()->GetAuthGameMode<ADemonGameMode>();
		if (DemonGameMode)
		{
			AttackerController->SetHUDDamageText();

			if (bHeadShot)
			{
				AttackerController->SetHUDHeadShot(true);
			}
			
			DemonPlayerController = DemonPlayerController == nullptr ? Cast<ADemonPlayerController>(Controller) : DemonPlayerController;

			DemonGameMode->PlayerEliminated(this, DemonPlayerController, AttackerController);
		}
	}

}

void ADemonCharacter::SetMeshHealthBar()
{
	HealthMaterial = HealthBarMesh->GetMaterial(0);
	if (HealthMaterial)
	{
		HealthDynamicMaterialInstance = UMaterialInstanceDynamic::Create(HealthMaterial, HealthBarMesh);	
		HealthBarMesh->SetMaterial(0, HealthDynamicMaterialInstance);	
	}
}

void ADemonCharacter::SetMeshShieldBar()
{
	ShieldMaterial = ShieldBarMesh->GetMaterial(0);
	if (ShieldMaterial)
	{
		ShieldDynamicMaterialInstance = UMaterialInstanceDynamic::Create(ShieldMaterial, ShieldBarMesh);
		ShieldBarMesh->SetMaterial(0, ShieldDynamicMaterialInstance);
	}
}

void ADemonCharacter::UpdateMeshHealthBar(float HealthMeshStat)
{
	if (HealthDynamicMaterialInstance)
	{
		HealthDynamicMaterialInstance->SetScalarParameterValue(TEXT("HealthMeshValue"), HealthMeshStat);
	}
}

void ADemonCharacter::UpdateMeshShieldBar(float ShieldMeshStat)
{
	if (ShieldDynamicMaterialInstance)
	{
		ShieldDynamicMaterialInstance->SetScalarParameterValue(TEXT("ShieldMeshValue"), ShieldMeshStat);
	}
}


void ADemonCharacter::PollInit()
{
	if (DemonPlayerState == nullptr)
	{
		DemonPlayerState = GetPlayerState<ADemonPlayerState>();
		if (DemonPlayerState)
		{
			DemonPlayerState->AddToScore(0.f);
			DemonPlayerState->AddToDeaths(0);

			ADemonGameState* DemonGameState = Cast<ADemonGameState>(UGameplayStatics::GetGameState(this));

			if (DemonGameState && DemonGameState->TopScoringPlayers.Contains(DemonPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
	if (DemonPlayerController == nullptr)
	{
		DemonPlayerController = DemonPlayerController == nullptr ? Cast<ADemonPlayerController>(Controller) : DemonPlayerController;
		if (DemonPlayerController)
		{
			SpawnDefaultWeapon();
			UpdateHUDAmmo();
			GetWorldTimerManager().SetTimer(SaveTimerHandle, this, &ADemonCharacter::SavePlayerLocation, 3, false);
		}
	}

}

void ADemonCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();


	//if (GEngine)
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("VALID!!"));


	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ADemonCharacter::Look(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();


	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		this->AddControllerYawInput(LookAxisVector.X);
		this->AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ADemonCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f) 
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ADemonCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ADemonCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ADemonCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ADemonCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{	
		ServerEquipButtonPressed();
	}
}

void ADemonCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
	}
}

void ADemonCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
	
}

void ADemonCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void ADemonCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ADemonCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

float ADemonCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ADemonCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	
	if (Speed == 0.f && !bIsInAir) //Standing still, Not Jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		bUseControllerRotationYaw = true;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);
	}
	

	if (Speed > 0.f || bIsInAir) // Running, or Jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ADemonCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map pitch from 270,360 to -90,0
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ADemonCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();

	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw)

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ADemonCharacter::Jump()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ADemonCharacter::Run()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	if (Combat)
	{
		
		bLocallyRunning = true;
		Combat->RunButtonPressed(true);
	}
}

void ADemonCharacter::StopRun()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		
		bLocallyRunning = false;
		Combat->RunButtonPressed(false);
	}
}

void ADemonCharacter::SwapButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		ServerSwapButtonPressed();
	}

}
void ADemonCharacter::ServerSwapButtonPressed_Implementation()
{
	if (Combat)
	{
		if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}


void ADemonCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		StopRun();
		Combat->FireButtonPressed(true);
	}
}

void ADemonCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ADemonCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 10.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}

	}
}

void ADemonCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ADemonCharacter::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}

void ADemonCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;

		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;

		}
	}
}

void ADemonCharacter::OnRep_Health(float LastHealth)
{
	MeshHealth = FMath::Clamp(Health * -1 / 100 + 0.5, -0.5f, MaxMeshHealth);
	UpdateMeshHealthBar(MeshHealth);

	if (Health < LastHealth)
	{	
		PlayHitReactMontage();	
	}
}

void ADemonCharacter::OnRep_Shield(float LastShield)
{
	MeshShield = FMath::Clamp(Shield * -1 / 100 + 0.5, -0.5f, MaxMeshShield);
	UpdateMeshShieldBar(MeshShield);

	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ADemonCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}

}

void ADemonCharacter::UpdateSpecialEffects()
{
	/*
	DemonPlayerController = DemonPlayerController == nullptr ? Cast<ADemonPlayerController>(Controller) : DemonPlayerController;
	if (DemonPlayerController)
	{
		DemonPlayerController->SetHUDSpeedLines(bLocallyRunning);
	}
	*/
}


bool ADemonCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ADemonCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

bool ADemonCharacter::IsRunning()
{
	if (Combat == nullptr) return false;
	return Combat->bRunning;
}

bool ADemonCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

AWeapon* ADemonCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ADemonCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState ADemonCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}







