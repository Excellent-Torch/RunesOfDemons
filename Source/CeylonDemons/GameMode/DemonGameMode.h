// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DemonGameMode.generated.h"

namespace MatchState
{
	extern CEYLONDEMONS_API const FName Cooldown; // Match Duration has been reached. Display winner and begin cooldown timer.
}

/**
 * 
 */
UCLASS()
class CEYLONDEMONS_API ADemonGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ADemonGameMode();
	virtual void Tick(float DeltaTime);
	virtual void PlayerEliminated(class ADemonCharacter* ElimmedCharacter, class ADemonPlayerController* VictimController, ADemonPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);
	
	void PlayerLeftGame(class ADemonPlayerState* PlayerLeaving);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 60.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 480.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 60.f;

	UPROPERTY()
	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	UPROPERTY()
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime;  }
};
