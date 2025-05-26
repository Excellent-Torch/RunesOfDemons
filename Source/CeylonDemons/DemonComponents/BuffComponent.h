// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CEYLONDEMONS_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ADemonCharacter;
	void Heal(float HealAmount, float HealingTime);
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	void BuffSpeed(bool SpeedBuff, float BuffTime);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);

	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void SetInitialJumpVelocity(float Velocity);
	void SpawnJumpDecals();

protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);

private:
	UPROPERTY()
	class ADemonCharacter* Character;

	// Health Buff
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	// Shield Buff
	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount = 0.f;

	// Speed Buff
	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;
	bool bSpeedBuff = false;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(bool SpeedBuff);


	//Jump Buff
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;
	bool bJumpBuff = false;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity, bool bBuffed);

	UPROPERTY(EditAnywhere, Category = "Jump")
	UMaterialInterface* JumpDecalMaterial;

	UPROPERTY(EditAnywhere, Category = "Jump")
	USoundBase* BuffJumpSound;

	UPROPERTY(EditAnywhere, Category = "Jump")
	float JumpDecalSize = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Jump")
	float JumpDecalLifeSpan = 2.0f;


public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE bool GetSpeedBuff() const { return bSpeedBuff; }
};
