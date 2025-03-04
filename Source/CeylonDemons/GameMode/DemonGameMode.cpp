// Fill out your copyright notice in the Description page of Project Settings.


#include "DemonGameMode.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "CeylonDemons/PlayerController/DemonPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "CeylonDemons/PlayerState/DemonPlayerState.h"
#include "CeylonDemons/GameState/DemonGameState.h"

namespace MatchState
{
	const FName  Cooldown = FName("Cooldown");
}

ADemonGameMode::ADemonGameMode()
{
	bDelayedStart = true;
}


void ADemonGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ADemonGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			bUseSeamlessTravel = false;
			GetWorld()->ServerTravel(FString("/Game/Maps/SecondDemoMap?listen"));
		}
	}

}

void ADemonGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ADemonPlayerController* DemonPlayer = Cast<ADemonPlayerController>(*It);
		if (DemonPlayer)
		{
			DemonPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ADemonGameMode::PlayerEliminated(ADemonCharacter* ElimmedCharacter, ADemonPlayerController* VictimController, ADemonPlayerController* AttackerController)
{
	ADemonPlayerState* AttackerPlayerState = AttackerController ? Cast<ADemonPlayerState>(AttackerController->PlayerState) : nullptr;
	ADemonPlayerState* VictimPlayerState = VictimController ? Cast<ADemonPlayerState>(VictimController->PlayerState) : nullptr;

	ADemonGameState* DemonGameState = GetGameState<ADemonGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		TArray<ADemonPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : DemonGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(1.f);
		DemonGameState->UpdateTopScore(AttackerPlayerState);
		if (DemonGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ADemonCharacter* Leader = Cast<ADemonCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!DemonGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ADemonCharacter* Loser = Cast<ADemonCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDeaths(1);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ADemonPlayerController* DemonPlayer = Cast<ADemonPlayerController>(*It);
		if (DemonPlayer && AttackerPlayerState && VictimPlayerState)
		{
			DemonPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void ADemonGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ADemonGameMode::PlayerLeftGame(class ADemonPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ADemonGameState* DemonGameState = GetGameState<ADemonGameState>();
	if (DemonGameState && DemonGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		DemonGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ADemonCharacter* CharacterLeaving = Cast<ADemonCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}
