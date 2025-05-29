#include "DestructionComponent.h"
#include "CeylonDemons/Character/DemonCharacter.h"


UDestructionComponent::UDestructionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;


}



void UDestructionComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void UDestructionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


