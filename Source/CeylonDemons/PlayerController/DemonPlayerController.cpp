// Fill out your copyright notice in the Description page of Project Settings.


#include "DemonPlayerController.h"
#include "CeylonDemons/HUD/DemonHUD.h"
#include "CeylonDemons/HUD/CharacterOverlay.h"
#include "CeylonDemons/HUD/Announcement.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "Net/UnrealNetwork.h"
#include "CeylonDemons/GameMode/DemonGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "CeylonDemons/DemonComponents/CombatComponent.h"
#include "CeylonDemons/GameState/DemonGameState.h"
#include "CeylonDemons/PlayerState/DemonPlayerState.h"
#include "Components/Image.h"
#include "CeylonDemons/HUD/ReturnToMainMenu.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "Sound/SoundCue.h"


void ADemonPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ADemonPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
		if (DemonHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				DemonHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				DemonHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				DemonHUD->AddElimAnnouncement("You", "yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				DemonHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			DemonHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}




void ADemonPlayerController::BeginPlay()
{
	Super::BeginPlay();


	ServerCheckMatchState();
	SetupInputMapping();

	DemonHUD = Cast<ADemonHUD>(GetHUD());
	if (DemonHUD)
	{
		DemonHUD->AddAnnouncemnet();
	}
}



void ADemonPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADemonPlayerController, MatchState);
}

void ADemonPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
}

void ADemonPlayerController::SetupInputMapping()
{
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	
}

void ADemonPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ADemonPlayerController::HighPingWarning()
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->HighPingImage &&
		DemonHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		DemonHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		DemonHUD->CharacterOverlay->PlayAnimation(
			DemonHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

void ADemonPlayerController::StopHighPingWarning()
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->HighPingImage &&
		DemonHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		DemonHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (DemonHUD->CharacterOverlay->IsAnimationPlaying(DemonHUD->CharacterOverlay->HighPingAnimation))
		{
			DemonHUD->CharacterOverlay->StopAnimation(DemonHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ADemonPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? TObjectPtr<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetPingInMilliseconds() > HighPingThreshold) 
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
			}
		}
		HighPingRunningTime = 0.f;
	}

	SetHUDPing();

	bool bHighPingAnimationPlaying =
		DemonHUD && DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->HighPingAnimation &&
		DemonHUD->CharacterOverlay->IsAnimationPlaying(DemonHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ADemonPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void ADemonPlayerController::ServerCheckMatchState_Implementation()
{
	ADemonGameMode* GameMode = Cast<ADemonGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{	
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);

		if (DemonHUD && MatchState == MatchState::WaitingToStart)
		{
			DemonHUD->AddAnnouncemnet();
		}
	}
}

void ADemonPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
}
void ADemonPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ADemonPlayerController::SetHUDScore(float Score)
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		DemonHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ADemonPlayerController::SetHUDDeaths(int32 Deaths)
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->DeathsAmount;
	if (bHUDValid)
	{
		FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		DemonHUD->CharacterOverlay->DeathsAmount->SetText(FText::FromString(DeathsText));
	}
	else
	{
		bInitializeDeaths = true;
		HUDDeaths = Deaths;
	}
}

void ADemonPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		DemonHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ADemonPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		DemonHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ADemonPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime <= 30.f)
		{
			/* -Turn timer red .. -Blink timer*/
			TimeRunningOut();
		}
		if (CountdownTime < 0.f)
		{
			DemonHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		DemonHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ADemonPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->Announcement &&
		DemonHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			DemonHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		DemonHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ADemonPlayerController::SetHUDPing()
{
	PlayerState = PlayerState == nullptr ? TObjectPtr<APlayerState>() : PlayerState;
	if (PlayerState)
	{
		float CurrentPing = PlayerState->GetPingInMilliseconds();
		if (DemonHUD && DemonHUD->CharacterOverlay && DemonHUD->CharacterOverlay->PingText && DemonHUD->CharacterOverlay->PingAmountText)
		{
			FString PingText = "Ping:";
			DemonHUD->CharacterOverlay->PingText->SetText(FText::FromString(PingText));
			FString PingAmountText = FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentPing));
			DemonHUD->CharacterOverlay->PingAmountText->SetText(FText::FromString(PingAmountText));
		}
	}
	
}
void ADemonPlayerController::SetHUDSpeedLines(bool bPressed)
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->SpeedLines &&
		DemonHUD->CharacterOverlay->SpeedLinesAnimation;
	
	if (bHUDValid && bPressed)
	{
		//DemonHUD->CharacterOverlay->SpeedLines->SetRenderOpacity(1.f);
		DemonHUD->CharacterOverlay->PlayAnimation(
			DemonHUD->CharacterOverlay->SpeedLinesAnimation,
			0.f);

	}
	else if (bHUDValid && DemonHUD->CharacterOverlay->IsAnimationPlaying(DemonHUD->CharacterOverlay->SpeedLinesAnimation))
	{	
		DemonHUD->CharacterOverlay->StopAnimation(DemonHUD->CharacterOverlay->SpeedLinesAnimation);	
	}
}

void ADemonPlayerController::SetHUDHitMark(bool bGettingHit)
{
	HitMark(bGettingHit);
}

void ADemonPlayerController::HitMark_Implementation(bool bGettingHit)
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->HitMark &&
		DemonHUD->CharacterOverlay->HitMarkAnimation;

	

	if (bHUDValid && bGettingHit)
	{
		DemonHUD->CharacterOverlay->HitMark->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
		DemonHUD->CharacterOverlay->HitMark->SetRenderOpacity(1.f);
		DemonHUD->CharacterOverlay->PlayAnimation(
			DemonHUD->CharacterOverlay->HitMarkAnimation,
			0.f);

	}	
}

void ADemonPlayerController::SetHUDDamageText()
{
	ClientDamageText();
}

void ADemonPlayerController::ClientDamageText_Implementation()
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	bool bHUDValid = DemonHUD &&
		DemonHUD->CharacterOverlay &&
		DemonHUD->CharacterOverlay->DamageText &&
		DemonHUD->CharacterOverlay->DamageTextAnimation &&
		DemonHUD->CharacterOverlay->HitMark &&
		DemonHUD->CharacterOverlay->HitMarkAnimation;

	ADemonCharacter* DemonCharacter = Cast<ADemonCharacter>(GetPawn());
	if (bHUDValid && DemonCharacter && DemonCharacter->GetDamageScore())
	{
		if (ElimSound)
		{
			UGameplayStatics::SpawnSoundAtLocation(
				GetWorld(),
				ElimSound,
				DemonCharacter->GetActorLocation()
			);
		}

		FString DamageText = FString::Printf(TEXT("+%d XP"), DemonCharacter->GetDamageScore());
		DemonHUD->CharacterOverlay->DamageText->SetText(FText::FromString(DamageText));

		DemonHUD->CharacterOverlay->DamageText->SetRenderOpacity(1.f);
		DemonHUD->CharacterOverlay->PlayAnimation(
			DemonHUD->CharacterOverlay->DamageTextAnimation,
			0.f);

		DemonHUD->CharacterOverlay->HitMark->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
		DemonHUD->CharacterOverlay->HitMark->SetRenderOpacity(1.f);
		DemonHUD->CharacterOverlay->PlayAnimation(
			DemonHUD->CharacterOverlay->HitMarkElimAnimation,
			0.f);

	}
}


void ADemonPlayerController::SetHUDTime()
{

	float TimeLeft = 0.f;

	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		DemonGameMode = DemonGameMode == nullptr ? Cast<ADemonGameMode>(UGameplayStatics::GetGameMode(this)) : DemonGameMode;
		if (DemonGameMode)
		{
			SecondsLeft = FMath::CeilToInt(DemonGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ADemonPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (DemonHUD && DemonHUD->CharacterOverlay)
		{
			CharacterOverlay = DemonHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDeaths) SetHUDDeaths(HUDDeaths);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
			}
		}
	}
}

void ADemonPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &ADemonPlayerController::ShowReturnToMainMenu);

}

void ADemonPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}
void ADemonPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
float ADemonPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ADemonPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ADemonPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}
void ADemonPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ADemonPlayerController::HandleMatchHasStarted()
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	if (DemonHUD)
	{

		if (DemonHUD->CharacterOverlay == nullptr) DemonHUD->AddCharacterOverlay();
		if (DemonHUD->Announcement)
		{
			DemonHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ADemonPlayerController::HandleCooldown()
{
	DemonHUD = DemonHUD == nullptr ? Cast<ADemonHUD>(GetHUD()) : DemonHUD;
	if (DemonHUD)
	{
		DemonHUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid = DemonHUD->Announcement &&
			DemonHUD->Announcement->AnnouncementText &&
			DemonHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			DemonHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			DemonHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ADemonGameState* DemonGameState = Cast<ADemonGameState>(UGameplayStatics::GetGameState(this));
			ADemonPlayerState* DemonPlayerState = GetPlayerState<ADemonPlayerState>();

			if (DemonGameState && DemonPlayerState)
			{
				TArray<ADemonPlayerState*> TopPlayers = DemonGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no Winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == DemonPlayerState)
				{
					InfoTextString = FString("You are the Winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("The Winner Is: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("It's a DRAW!:\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}


				DemonHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	ADemonCharacter* DemonCharacter = Cast<ADemonCharacter>(GetPawn());
	if (DemonCharacter && DemonCharacter->GetCombat())
	{
		DemonCharacter->bDisableGameplay = true;
		DemonCharacter->GetCombat()->FireButtonPressed(false);
	}
}

// Custom Code (Experimental)

void ADemonPlayerController::BlinkTimerFinished()
{
	DemonHUD->CharacterOverlay->MatchCountdownText->SetVisibility(ESlateVisibility::Hidden);
}

void ADemonPlayerController::TimeRunningOut()
{
	DemonHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
	DemonHUD->CharacterOverlay->MatchCountdownText->SetVisibility(ESlateVisibility::Visible);

	GetWorldTimerManager().SetTimer(BlinkTimer, this, &ADemonPlayerController::BlinkTimerFinished, 0.75f);
}