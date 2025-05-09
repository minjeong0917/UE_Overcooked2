// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Global/OC2Struct.h"
#include "Global/OC2Enum.h"
#include "Global/OC2Const.h"

#include "SpawnManageComponent.generated.h"

class AIngredient;
class APlate;
class APot;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OVERCOOKED2_API USpawnManageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USpawnManageComponent();

	AIngredient* SpawnIngredientActor(TSubclassOf<AIngredient> IngredientToSpawn, EIngredientType IngredientType);
	APlate* SpawnPlateActor(TSubclassOf<APlate> PlateTopSpawn, EPlateState PlateState);
	APot* SpawnPotActor(TSubclassOf<APot> PotToSpawn, EPotState PotState);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

};
