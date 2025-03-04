#include "LagComponent.h"
#include "CeylonDemons/Character/DemonCharacter.h"
#include "Components/BoxComponent.h"
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
		if (Character->GetMesh() == nullptr || Character->GetMesh()->GetPhysicsAsset() == nullptr) return;
		UPhysicsAsset* PhysicsAsset = Character->GetMesh()->GetPhysicsAsset();
		if (!PhysicsAsset) return;

		
		for (const auto& BodySetup : PhysicsAsset->SkeletalBodySetups)
		{
			if (!BodySetup) continue;

			FName BoneName = BodySetup->BoneName;
			int32 BoneIndex = Character->GetMesh()->GetBoneIndex(BoneName);
			if (BoneIndex == INDEX_NONE) continue;

			FTransform BoneTransform = Character->GetMesh()->GetBoneTransform(BoneIndex);

			Package.Time = GetWorld()->GetTimeSeconds();
			for (const auto& CapsuleElem : BodySetup->AggGeom.SphylElems)
			{
				FCapsuleInformation CapsuleInformation;

				CapsuleInformation.Location = CapsuleElem.GetTransform().GetLocation();
				CapsuleInformation.Rotation = CapsuleElem.Rotation;
				CapsuleInformation.HalfHeight = CapsuleElem.GetScaledHalfLength(BodySetup->BuildScale3D);
				CapsuleInformation.Radius = CapsuleElem.GetScaledRadius(BodySetup->BuildScale3D);

				Package.HitBoxInfo.Add(CapsuleElem.GetName(), CapsuleInformation);

				DrawDebugCapsule(
					GetWorld(),
					BoneTransform.GetLocation(),
					CapsuleElem.GetScaledHalfLength(BodySetup->BuildScale3D),
					CapsuleElem.GetScaledRadius(BodySetup->BuildScale3D),
					BoneTransform.GetRotation(),
					FColor::Red,
					true
				);
			}
		}
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
			FColor::Red,
			true
		);
	}

}

