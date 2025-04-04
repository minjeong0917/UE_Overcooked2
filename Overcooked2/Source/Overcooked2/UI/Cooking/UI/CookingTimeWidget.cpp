// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Cooking/UI/CookingTimeWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UCookingTimeWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (true == bIsStart)
	{
		StartTimer(DeltaTime);
	}
	else
	{
		CurTime = TotalTime;
	}
}


void UCookingTimeWidget::StartTimer(float DeltaTime)
{

	int Min = static_cast<int>(CurTime / 60);
	int Sec = static_cast<int>(CurTime) % 60;

	if (Min <= 0 && Sec <= 0)
	{
		Min = 0;
		Sec = 0;
	}


	FString MinStr = "0";
	FString SecStr = "0";

	if (Min < 10)
	{
		MinStr = "0" + FString::FromInt(Min);
	}
	else
	{
		MinStr = FString::FromInt(Min);
	}



	if (Sec < 10)
	{
		SecStr = "0" + FString::FromInt(Sec);
	}
	else
	{
		SecStr = FString::FromInt(Sec);
	}


	Time->SetText(FText::FromString(MinStr + ":" + SecStr));
	TimeProgressBar->SetPercent(CurTime / TotalTime);

	FLinearColor Color = TimeProgressBar->GetFillColorAndOpacity();

	float TimeLimit = 180.0f;

	if (CurTime > (TimeLimit / 3) * 2)
	{
		TimeProgressBar->SetFillColorAndOpacity({ Color.R, Color.G, 0.0f });
	}	
	else if (CurTime > TimeLimit / 3)
	{
		TimeProgressBar->SetFillColorAndOpacity({ Color.R + DeltaTime * 0.01f, Color.G + DeltaTime * 0.01f, 0.0f });
	}
	else 
	{
		TimeProgressBar->SetFillColorAndOpacity({ Color.R + DeltaTime * 0.01f, Color.G - DeltaTime * 0.02f, 0.0f });
	}


	CurTime -= DeltaTime;

}