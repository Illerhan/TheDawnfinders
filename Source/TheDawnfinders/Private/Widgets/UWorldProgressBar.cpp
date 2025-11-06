// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/UWorldProgressBar.h"

void UWorldProgressBar::Show_Implementation(float CurrentValue)
{
	StartValue = CurrentValue;
	IsDisplayed = true;

}

void UWorldProgressBar::ActualiseProgress_Implementation(float CurrentValue)
{

}


void UWorldProgressBar::Hide_Implementation()
{
	IsDisplayed = false;
}