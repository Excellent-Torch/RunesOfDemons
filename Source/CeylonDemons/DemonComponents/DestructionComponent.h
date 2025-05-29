// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DestructionComponent.generated.h"


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CEYLONDEMONS_API UDestructionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDestructionComponent();
	friend class ADemonCharacter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void FireDestruction();

	UPROPERTY(EditAnywhere, Category = "Destruction")
	UDataAsset* SurfaceResponseDA;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Destruction")
	float ParticleAmtModulation = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Destruction")
	FHitResult DestructionHitResult;

private:
	UPROPERTY()
	class ADemonCharacter* Character;


		
};
