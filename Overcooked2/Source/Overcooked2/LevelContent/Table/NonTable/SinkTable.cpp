// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelContent/Table/NonTable/SinkTable.h"
#include "LevelContent/Cook/Plate.h"
#include <Character/OC2Character.h>
#include "Components/WidgetComponent.h"
#include <LevelContent/Cook/Widget/GaugeTextureWidget.h>
#include <Global/Component/TimeEventComponent.h>
#include <Net/UnrealNetwork.h>
#include <Global/OC2Global.h>
#include <Global/Data/OC2GlobalData.h>
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

ASinkTable::ASinkTable()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	ProgressBarComponent = CreateDefaultSubobject<UWidgetComponent>("ProgressBar");
	ProgressBarComponent->SetupAttachment(RootComponent);

	CleanPlateComponent = CreateDefaultSubobject<USceneComponent>("CleanPlate");
	CleanPlateComponent->SetupAttachment(RootComponent);

	{
		ComponentForDishes1 = CreateDefaultSubobject<USceneComponent>("DirtyPlate1");
		ComponentForDishes1->SetupAttachment(ComponentForCooking);
		DirtyPlateComponents.Add(ComponentForDishes1);

		ComponentForDishes2 = CreateDefaultSubobject<USceneComponent>("DirtyPlate2");
		ComponentForDishes2->SetupAttachment(ComponentForCooking);
		DirtyPlateComponents.Add(ComponentForDishes2);

		ComponentForDishes3 = CreateDefaultSubobject<USceneComponent>("DirtyPlate3");
		ComponentForDishes3->SetupAttachment(ComponentForCooking);
		DirtyPlateComponents.Add(ComponentForDishes3);

		ComponentForDishes4 = CreateDefaultSubobject<USceneComponent>("DirtyPlate4");
		ComponentForDishes4->SetupAttachment(ComponentForCooking);
		DirtyPlateComponents.Add(ComponentForDishes4);
	}

	TimeEventComponent = CreateDefaultSubobject<UTimeEventComponent>(TEXT("TimeEventComponent"));

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("SoundFx");
	AudioComponent->SetupAttachment(RootComponent);
}

void ASinkTable::BeginPlay()
{
	Super::BeginPlay();

	InitProgressBar();

	SoundEffectPlate = UOC2GlobalData::GetTableBaseSound(GetWorld(), "PlatePlaced");
	SoundEffectWashing = UOC2GlobalData::GetTableBaseSound(GetWorld(), "Washing");
	AudioComponent->SetSound(SoundEffectWashing);
}

void ASinkTable::InitProgressBar()
{
	// 위젯 클래스 지정
	ProgressBarComponent->SetWidgetClass(TSubClassWidget); // WBP 위젯으로 설정
	UUserWidget* UserWidget = ProgressBarComponent->GetUserWidgetObject();
	if (nullptr != UserWidget)
	{
		WidgetPtr = Cast<UGaugeTextureWidget>(UserWidget);
		WidgetPtr->SetPosition(FVector2D{ 5.0f, 10.0f });
	}
	ProgressBarComponent->SetDrawAtDesiredSize(true);   // 위젯의 실제 크기로 렌더
	ProgressBarComponent->SetPivot(FVector2D(0.5f, 0.5f)); // 중심 정렬
	ProgressBarComponent->SetWidgetSpace(EWidgetSpace::Screen); // 월드 공간에서 3D처럼 보이게
	ProgressBarComponent->bHiddenInGame = true;

	// 카메라를 향하도록 설정
	ProgressBarComponent->SetTwoSided(true);
	ProgressBarComponent->SetTickWhenOffscreen(true);
	ProgressBarComponent->SetWorldLocation(this->GetActorLocation());
}

ACooking* ASinkTable::Interact(AActor* ChefActor)
{
	// 설거지된 접시가 하나 이상 있으면 캐릭터에게 하나를 줄 수 있다.

	if (true == HasAuthority())
	{
		if (true == CleanPlates.IsEmpty())
		{
			return nullptr;
		}
		CleanPlates.Last()->BaseFromSelf();
		CookingPtr = CleanPlates.Pop();
	
		return CookingPtr;
	}

	return nullptr;
}

// 접시가 싱크대로 들어오는 로직
void ASinkTable::PlaceItem(ACooking* ReceivedCooking)
{
	PlacePlates(ReceivedCooking);
}

// 접시가 싱크대로 들어오는 로직 (서버만 배열을 관리한다.)
void ASinkTable::PlacePlates_Implementation(ACooking* ReceivedCooking)
{
	if (false == HasAuthority())
	{
		return;
	}
	if (nullptr == ReceivedCooking)
	{
		return;
	}

	if (ECookingType::ECT_PLATE == ReceivedCooking->GetCookingType())
	{
		APlate* TempPlate = Cast<APlate>(ReceivedCooking);
		if (nullptr == TempPlate)
		{
			return;
		}

		// 1. 더티 플레이트만 들어올 수 있다.
		if (true == TempPlate->IsDirtyPlate())
		{
			for (int32 i = 0; i < TempPlate->Plates.Num(); i++)
			{
				TempPlate->Plates[i]->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				DirtyPlates.Add(TempPlate->Plates[i]);
			}

			TempPlate->Plates.Empty();
		}

		for (int32 i = 0; i < DirtyPlates.Num(); i++)
		{
			DirtyPlates[i]->AttachToComponent(DirtyPlateComponents[i], FAttachmentTransformRules::KeepRelativeTransform);
			DirtyPlates[i]->SetActorLocation(DirtyPlateComponents[i]->GetComponentLocation());
			DirtyPlates[i]->SetActorRotation(DirtyPlateComponents[i]->GetComponentRotation());
			DirtyPlates[i]->SetActorScale3D(PlateSize * 0.6);
		}
	}

}

void ASinkTable::DoTheDishes(AOC2Character* ChefActor)
{
	ChefPtr = Cast<AOC2Character>(ChefActor);

	// 셰프가 싱크대를 떠나지 않았고 설거지할 접시가 남아있다면
	if (nullptr != ChefActor && 0 < DirtyPlates.Num())
	{
		ChefPtr->Washing(true);

		Timer = 0.0f;
		bTimerActivated = true;

		HideProgressBar(false);
	}

	bAudioActive = true;
	if (nullptr != AudioComponent && nullptr != SoundEffectWashing)
	{
		AudioComponent->Play();
	}
}

void ASinkTable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		// 셰프가 나와의 상호작용을 중단하고 떠났는지
		CheckChefIsWashing();

		UpdateProgressBar(DeltaTime);

		WashingIsDone();

		RepeatWashing();
	}
}

void ASinkTable::CheckChefIsWashing_Implementation()
{

	// 셰프가 나와 상호작용을 하던 중에
	if (nullptr != ChefPtr)
	{
		// 다른 행동으로 상호작용이 중단되었다면
		if (false == ChefPtr->IsWashing())
		{
			bTimerActivated = false;
			bAudioActive = false;
			if (nullptr != AudioComponent && nullptr != SoundEffectWashing)
			{
				AudioComponent->Stop();
			}

			ChefPtr = nullptr;
			HideProgressBar(true);
		}
	}
}

void ASinkTable::FinishWashing_Implementation()
{
	bWashingDone = true;
}

void ASinkTable::UpdateProgressBar/*_Implementation*/(float DeltaTime)
{
	if (true == bTimerActivated)
	{
		Timer += DeltaTime;

		if (Timer > 2.0f)
		{
			// 트리거 변수
			FinishWashing();
		}
	}

	Ratio = (Timer / 0.4f) * 0.2f;
	WidgetPtr->SetProgressTimeRatio(Ratio);
}

// 설거지 완료했을 때 한 번 호출, 싱크대로 접시를 하나 이동시킨다.
void ASinkTable::WashingIsDone_Implementation()
{
	if (false == HasAuthority())
	{
		return;
	}

	// 설거지 중이라면 리턴
	if (false == bWashingDone)
	{
		return;
	}

	bTimerActivated = false; // 타이머 끄고

	bAudioActive = false;
	if (nullptr != AudioComponent && nullptr != SoundEffectWashing)
	{
		AudioComponent->Stop();
	}

	if (false == DirtyPlates.IsEmpty())
	{
		CleanPlates.Add(DirtyPlates.Pop());
		APlate* NewCleanPlate = CleanPlates.Last();
		NewCleanPlate->SetPlateState(EPlateState::EMPTY);
	}

	int Result = CleanPlates.Num();

	HideProgressBar(true);

	CleanPlates.Last()->AttachToComponent(CleanPlateComponent, FAttachmentTransformRules::KeepRelativeTransform);
	CleanPlates.Last()->SetActorLocation(CleanPlateComponent->GetComponentLocation());
	CleanPlates.Last()->AddActorWorldOffset(FVector::UnitZ()* 10.0f * (CleanPlates.Num() - 1));
	CleanPlates.Last()->SetActorScale3D(PlateSize);

	if (nullptr != SoundEffectPlate)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SoundEffectPlate);
	}

}

void ASinkTable::HideProgressBar_Implementation(bool Value)
{
	ProgressBarComponent->SetHiddenInGame(Value);
}

void ASinkTable::RepeatWashing_Implementation()
{
	// 1. 설거지가 끝났을 때 아직 설거지 할 접시가 남아있다면 다시 설거지 로직 호출
	if (true == bWashingDone && 0 < DirtyPlates.Num())
	{
		DoTheDishes(ChefPtr); // 셰프가 처음에 한 번 불러준다. 이후에는 내가 불러줌
		bWashingDone = false;
		return;
	}
	else if (true == bWashingDone)
	{
		// 설거지 모션 중단
		ChefPtr->Washing(false);
		bWashingDone = false;
		return;
	}
}

void ASinkTable::SetAttachToDirtyPlate()
{
	for (size_t i = 0; i < DirtyPlates.Num(); i++)
	{
		FVector Result = DirtyPlateComponents[i]->GetComponentLocation();
		//DirtyPlates[i]->SetActorRotation(DirtyPlateComponents[i]->GetComponentRotation());
		DirtyPlates[i]->AttachToComponent(DirtyPlateComponents[i], FAttachmentTransformRules::KeepRelativeTransform);
		DirtyPlates[i]->SetActorLocation(DirtyPlateComponents[i]->GetComponentLocation());
	}
}

void ASinkTable::OnRep_ChangeProgress()
{
	WidgetPtr->SetProgressTimeRatio(Ratio);
}

void ASinkTable::OnRep_ControlAudioComponent()
{
	if (nullptr != AudioComponent && nullptr != SoundEffectWashing)
	{
		if (true == bAudioActive)
		{
			AudioComponent->Play();
		}
		else
		{
			AudioComponent->Stop();
		}
	}
}

void ASinkTable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASinkTable, ChefPtr);
	DOREPLIFETIME(ASinkTable, Timer);
	DOREPLIFETIME(ASinkTable, bTimerActivated);
	DOREPLIFETIME(ASinkTable, bWashingDone);
	DOREPLIFETIME(ASinkTable, Ratio);
	DOREPLIFETIME(ASinkTable, ComponentForDishes1);
	DOREPLIFETIME(ASinkTable, ComponentForDishes2);
	DOREPLIFETIME(ASinkTable, ComponentForDishes3);
	DOREPLIFETIME(ASinkTable, ComponentForDishes4);
	DOREPLIFETIME(ASinkTable, CleanPlateComponent);
	DOREPLIFETIME(ASinkTable, DirtyPlateComponents);
	DOREPLIFETIME(ASinkTable, ProgressBarComponent);
	DOREPLIFETIME(ASinkTable, bAudioActive);
}