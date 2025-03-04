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
protected:

	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);

private:

	UPROPERTY()
	ADemonCharacter* Character;

	UPROPERTY()
	class ADemonPlayerController* Controller;


};
