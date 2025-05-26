#include "BuffComponent.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "CeylonDemons/DemonComponents/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);

}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;

	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));

	Character->UpdateMeshHealthBar(Character->GetHealth() * -1 / 100 + 0.5);

	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield || Character == nullptr || Character->IsElimmed()) return;

	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	
	Character->UpdateMeshShieldBar(Character->GetShield() * -1 / 100 + 0.5);

	ShieldReplenishAmount -= ReplenishThisFrame;

	if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}
}

void UBuffComponent::BuffSpeed(bool SpeedBuff, float BuffTime)
{
	if (Character == nullptr) return;

	bSpeedBuff = SpeedBuff;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime
	);

	if (Character->GetCombat())
	{
		Character->GetCombat()->SetSpeedBuff(bSpeedBuff);
	}
	
	MulticastSpeedBuff(bSpeedBuff);
	
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::ResetSpeeds()
{
	if (Character == nullptr) return;

	bSpeedBuff = false;
	if (Character->GetCombat())
	{
		Character->GetCombat()->RunButtonPressed(false);
		Character->GetCombat()->SetSpeedBuff(bSpeedBuff);
	}
	MulticastSpeedBuff(bSpeedBuff);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(bool SpeedBuff)
{
	if (Character->GetCombat())
	{
		Character->GetCombat()->RunButtonPressed(false);
		Character->GetCombat()->SetSpeedBuff(SpeedBuff);
	}
}


void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;

	bJumpBuff = true;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity, bJumpBuff);
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::SpawnJumpDecals()
{
	if (Character == nullptr) return;
	if (!JumpDecalMaterial || !BuffJumpSound) return;

	FVector LeftFootLocation = Character->GetMesh()->GetSocketLocation(FName("LeftFoot"));
	FVector RightFootLocation = Character->GetMesh()->GetSocketLocation(FName("RightFoot"));

	FRotator DecalRotation = FRotator(-90, 0, 0); // Faces decal upward from the foot
	FVector DecalExtent = FVector(JumpDecalSize, JumpDecalSize, JumpDecalSize);

	// Jump Buff Sound
	UGameplayStatics::SpawnSoundAtLocation(
		GetWorld(),
		BuffJumpSound,
		LeftFootLocation
	);

	// Left Foot Decal
	UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(),
		JumpDecalMaterial,
		DecalExtent,
		LeftFootLocation,
		DecalRotation,
		JumpDecalLifeSpan
	);

	// Right Foot Decal
	UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(),
		JumpDecalMaterial,
		DecalExtent,
		RightFootLocation,
		DecalRotation,
		JumpDecalLifeSpan
	);
}

void UBuffComponent::ResetJump()
{
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity, false);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity, bool bBuffed)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
		bJumpBuff = bBuffed;
	}
}