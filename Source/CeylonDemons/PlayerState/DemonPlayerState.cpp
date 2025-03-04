// Fill out your copyright notice in the Description page of Project Settings.


#include "DemonPlayerState.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "CeylonDemons/PlayerController/DemonPlayerController.h"
#include "Net/UnrealNetwork.h"

void ADemonPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADemonPlayerState, Deaths);
}

void ADemonPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	Character = Character == nullptr ? Cast<ADemonCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ADemonPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}



void ADemonPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ADemonCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ADemonPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	 }
}

void ADemonPlayerState::AddToDeaths(int32 DeathsAmount)
{
	Deaths += DeathsAmount;

	Character = Character == nullptr ? Cast<ADemonCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ADemonPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDeaths(Deaths);
		}
	}
}

void ADemonPlayerState::OnRep_Deaths()
{
	Character = Character == nullptr ? Cast<ADemonCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ADemonPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDeaths(Deaths);
		}
	}
}

