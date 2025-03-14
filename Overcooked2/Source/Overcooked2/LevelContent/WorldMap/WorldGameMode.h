// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LevelContent/WorldMap/WorldMapData.h"
#include "WorldGameMode.generated.h"

/**
 * 
 */
UCLASS()
class OVERCOOKED2_API AWorldGameMode : public AGameMode
{
	GENERATED_BODY()

public:

protected:
	void BeginPlay() override;
	void Tick(float _DeltaTime) override;

private:
	void OpenRoad1_1(int _Idx);
	void ShowStone1();
	void ShowBuilding1_1();

	UPROPERTY()
	TArray<AActor*> ActorsChangingScale3D;

	UPROPERTY()
	TArray<AActor*> ActorsChangingVisibility;

	UPROPERTY()
	TArray<AActor*> ActorsChangingScaleY;

	UPROPERTY()
	class ATileGrid* Tile1_1 = nullptr;

	UPROPERTY()
	bool IsOpen1_1 = false;

	// Temp
	bool IsBuildingShow1_1 = false;
	int IdxOpenRoad = -1;

	EStageState CurStageState = EStageState::None;

	UPROPERTY()
	TObjectPtr<AActor> FocusCamera = nullptr;
};
