// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagComponent.generated.h"

USTRUCT(BlueprintType)
struct FCapsuleInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	float HalfHeight;

	UPROPERTY()
	float Radius;


};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FCapsuleInformation> HitBoxInfo;

	UPROPERTY()
	ADemonCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CEYLONDEMONS_API ULagComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagComponent();
	friend class ADemonCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	FServerSideRewindResult ServerSideRewind(class ADemonCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ADemonCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, class AWeapon* DamageCauser);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& NewerFrame, float HitTime);
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ADemonCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

	void CacheBoxPositions(ADemonCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ADemonCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ADemonCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ADemonCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	
private:

	UPROPERTY()
	ADemonCharacter* Character;

	UPROPERTY()
	class ADemonPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

};
