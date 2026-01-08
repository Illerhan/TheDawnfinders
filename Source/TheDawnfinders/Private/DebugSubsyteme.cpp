// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugSubsyteme.h"


void UDebugSubsyteme::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("--- DebugSubsyteme Initialized ---"));
	if (!DebugWindow.IsValid())
	{
		DebugWindow = MakeShareable(new FDebugWindow());
	}
	
}
void UDebugSubsyteme::Deinitialize()
{
	// C'est ici qu'on évite le crash. On détruit la fenêtre AVANT que le monde ne disparaisse.
	if (DebugWindow.IsValid())
	{
		// Si ta classe FDebugWindow a une méthode genre .Close() ou .Destroy(), appelle-la ici.
		// Sinon, le simple fait de reset le pointeur devrait suffire si le SlateIMWindowBase est bien fait.
		DebugWindow.Reset(); 
	}

	Super::Deinitialize();
}