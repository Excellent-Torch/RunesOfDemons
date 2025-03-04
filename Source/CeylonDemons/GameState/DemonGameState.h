// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DemonGameState.generated.h"

/**
 * 
 */
UCLASS()
class CEYLONDEMONS_API ADemonGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	void UpdateTopScore(class ADemonPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<class ADemonPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
};
