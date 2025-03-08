// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class CEYLONDEMONS_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* ScoreAmountAnimation;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeathsAmount;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* DeathAmountAnimation;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingAmountText;

	UPROPERTY(meta = (BindWidget))
	class UImage* HitMark;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HitMarkAnimation;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HitMarkElimAnimation;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DamageText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* DamageTextAnimation;

	UPROPERTY(meta = (BindWidget))
	UImage* SpeedLines;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* SpeedLinesAnimation;

};
