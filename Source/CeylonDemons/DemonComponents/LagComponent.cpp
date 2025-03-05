#include "LagComponent.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"


#include "DrawDebugHelpers.h"

ULagComponent::ULagComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;


}

void ULagComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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
		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagComponent::BeginPlay()
{
	Super::BeginPlay();

	FFramePackage Package;
	SaveFramePackage(Package);
	ShowFramePackage(Package, FColor::Orange);
}

void ULagComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ADemonCharacter>(GetOwner()) : Character;
	
	if (Character)
	{

		for (auto& HitBoxInfo : Character->HitBoxInfo)
		{
			FCapsuleInformation CapsuleInformation;
			//BoneTransform.TransformPosition(CapsuleElem.Center);
			CapsuleInformation.Location = HitBoxInfo.Value->GetComponentLocation();
			CapsuleInformation.Rotation = HitBoxInfo.Value->GetComponentRotation();
			CapsuleInformation.HalfHeight = HitBoxInfo.Value->GetScaledCapsuleHalfHeight();
			CapsuleInformation.Radius = HitBoxInfo.Value->GetScaledCapsuleRadius();
			
		

			Package.HitBoxInfo.Add(HitBoxInfo.Key, CapsuleInformation);
		}
		
	}
}

void ULagComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (const auto& BoxInfo : Package.HitBoxInfo)
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

