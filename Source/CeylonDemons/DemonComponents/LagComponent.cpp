#include "LagComponent.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "CeylonDemons/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"

ULagComponent::ULagComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;


}

void ULagComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackage();
}

void ULagComponent::BeginPlay()
{
	Super::BeginPlay();

}

FFramePackage ULagComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& NewerFrame, float HitTime)
{
	const float Distance = NewerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& NewerPair : NewerFrame.HitBoxInfo)
	{
		const FName& CapsuleInfoName = NewerPair.Key;
		const FCapsuleInformation& OlderBox = OlderFrame.HitBoxInfo[CapsuleInfoName];
		const FCapsuleInformation& NewerBox = NewerFrame.HitBoxInfo[CapsuleInfoName];

		FCapsuleInformation InterpBoxInfo;

		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, NewerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, NewerBox.Rotation, 1.f, InterpFraction);

		InterpBoxInfo.HalfHeight = NewerBox.HalfHeight;
		InterpBoxInfo.Radius = NewerBox.Radius;

		InterpFramePackage.HitBoxInfo.Add(CapsuleInfoName, InterpBoxInfo);
	}
	return FFramePackage();
}

FServerSideRewindResult ULagComponent::ConfirmHit(const FFramePackage& Package, ADemonCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// Enable collision for the head first
	UCapsuleComponent* HeadCapsule = HitCharacter->HitCapsuleCollisions[FName("J_Bip_C_Head")];
	HeadCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	FHitResult ConfirmHitResult;

	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_Visibility
		);

		if (ConfirmHitResult.bBlockingHit) // Return early if its a HeadShot
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

			return FServerSideRewindResult{ true, true };
		}
		else // didn't hit head, check the rest of the Capsules
		{
			for (auto& HitBoxPair : HitCharacter->HitCapsuleCollisions)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
				}
			}

			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECollisionChannel::ECC_Visibility
			);

			if (ConfirmHitResult.bBlockingHit)
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

				return FServerSideRewindResult{ true, false };
			}
		}
	}
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

	return FServerSideRewindResult{ false, false };
}

void ULagComponent::ServerScoreRequest_Implementation(ADemonCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character && HitCharacter && DamageCauser && Confirm.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageCauser->GetDamage(),
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}

void ULagComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugCapsule(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.HalfHeight,
			BoxInfo.Value.Radius,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}

}

FServerSideRewindResult ULagComponent::ServerSideRewind(ADemonCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagComponent() == nullptr ||
		HitCharacter->GetLagComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagComponent()->FrameHistory.GetTail() == nullptr;

	if (bReturn) return FServerSideRewindResult();

	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;

	// Frame history of the HitCharacter
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagComponent()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	if (OldestHistoryTime > HitTime)
	{
		// Too much lag!
		return FServerSideRewindResult();
	}

	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}

	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Newer = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Newer;

	while (Older->GetValue().Time > HitTime) // is Older still Newer than HitTime?
	{
		// Go back until: OlderTime < HitTime < YoungerTime
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();

		if (Older->GetValue().Time > HitTime)
		{
			Newer = Older;
		}
	}

	if (Older->GetValue().Time == HitTime) // If FrameToCheck is already found!
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if (bShouldInterpolate)
	{
		// Interpolate between Newer and Older
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Newer->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

void ULagComponent::CacheBoxPositions(ADemonCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCapsuleCollisions)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FCapsuleInformation CapsuleInformation;

			CapsuleInformation.Location = HitBoxPair.Value->GetComponentLocation();
			CapsuleInformation.Rotation = HitBoxPair.Value->GetComponentRotation();
			CapsuleInformation.HalfHeight = HitBoxPair.Value->GetScaledCapsuleHalfHeight();
			CapsuleInformation.Radius = HitBoxPair.Value->GetScaledCapsuleRadius();

			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, CapsuleInformation);
		}
	}
}

void ULagComponent::MoveBoxes(ADemonCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCapsuleCollisions)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);

			HitBoxPair.Value->SetCapsuleHalfHeight(Package.HitBoxInfo[HitBoxPair.Key].HalfHeight);
			HitBoxPair.Value->SetCapsuleRadius(Package.HitBoxInfo[HitBoxPair.Key].Radius);
		}
	}
}

void ULagComponent::ResetHitBoxes(ADemonCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCapsuleCollisions)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);

			HitBoxPair.Value->SetCapsuleHalfHeight(Package.HitBoxInfo[HitBoxPair.Key].HalfHeight);
			HitBoxPair.Value->SetCapsuleRadius(Package.HitBoxInfo[HitBoxPair.Key].Radius);

			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagComponent::EnableCharacterMeshCollision(ADemonCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

void ULagComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ADemonCharacter>(GetOwner()) : Character;

	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for (auto& HitBoxInfo : Character->HitCapsuleCollisions)
		{
			FCapsuleInformation CapsuleInformation;

			CapsuleInformation.Location = HitBoxInfo.Value->GetComponentLocation();
			CapsuleInformation.Rotation = HitBoxInfo.Value->GetComponentRotation();
			CapsuleInformation.HalfHeight = HitBoxInfo.Value->GetScaledCapsuleHalfHeight();
			CapsuleInformation.Radius = HitBoxInfo.Value->GetScaledCapsuleRadius();

			Package.HitBoxInfo.Add(HitBoxInfo.Key, CapsuleInformation);
		}
	}
}

void ULagComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		ShowFramePackage(ThisFrame, FColor::Red);
	}
}

