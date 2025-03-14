// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileGrid.generated.h"

USTRUCT(BlueprintType)
struct FTileData
{
	GENERATED_BODY()

	UPROPERTY()
	UInstancedStaticMeshComponent* TileInst = nullptr;

	float PrevAlpha = 0.f;
	float AmplitudeZ = 180.f / 5.f * 1.5f;
	bool IsFinishedAnim = false;
};

UCLASS()
class OVERCOOKED2_API ATileGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	ATileGrid();

	virtual void Tick(float DeltaTime) override;

	bool IsEndTransition() const;

	bool IsStarted = false;	// Temp

protected:
	virtual void BeginPlay() override;

private:
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "ISM", meta = (AllowPrivateAccess = "true"))
	int NumI = 0;

	UPROPERTY(EditAnywhere, Category = "ISM", meta = (AllowPrivateAccess = "true"))
	int NumJ = 0;

	float InitZ = 0.f;

	const float RADIUS = 57.25f;
	float MulRadius = 0.f;
	float ElpasedSecs = 0.f;
	float ElpasedRotateSecs = 0.f;

	int RotateIdx = -1;
	float AccumulatedX = 0.f;
	bool IsAllEnd = false;

	UPROPERTY()
	TMap<int8, FTileData> Tiles;
	TMap<FIntVector2, int8> OrderMap;

	FIntVector2 GetXY(int _IdxI, int _IdxJ);
	bool IsExclude(int _i, int _j);
	TArray<FIntVector2> GetSurroundedNewIdxs(TArray<FIntVector2>& _Centers, int _Idx);
	TMap<FIntVector2, int8> GetSurroundedIdxs(const FIntVector2& _RowCol, uint8 _Size);

	void CreateTiles(TMap<int8, FTileData>& _RefMap, int _Size);
	void SetStaticMeshes(TMap<int8, FTileData>& _Tiles);
	void SetTileMaterials(TMap<int8, FTileData>& _Tiles);
};
