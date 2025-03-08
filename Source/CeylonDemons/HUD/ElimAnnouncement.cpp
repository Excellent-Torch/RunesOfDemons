// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"
#include "Components/TextBlock.h"

void UElimAnnouncement::SetElimAnnouncementText(FString AttackerName, FString VictimName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s Killed %s!"), *AttackerName, *VictimName);
	if (AnnouncementText && AnnouncementTextAnimation)
	{
		AnnouncementText->SetText(FText::FromString(ElimAnnouncementText));

		this->PlayAnimation(
			AnnouncementTextAnimation,
			0.f);
	}
}