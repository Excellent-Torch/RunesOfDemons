// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DemonPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class CEYLONDEMONS_API ADemonPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/**
	* Replication Notifies
	*/
	virtual void OnRep_Score() override;
	
	UFUNCTION()
	virtual void OnRep_Deaths();

	void AddToScore(float ScoreAmount);
	void AddToDeaths(int32 DeathsAmount);

private:
	UPROPERTY()
	class ADemonCharacter* Character;
	UPROPERTY()
	class ADemonPlayerController* Controller;
	
	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;
};
