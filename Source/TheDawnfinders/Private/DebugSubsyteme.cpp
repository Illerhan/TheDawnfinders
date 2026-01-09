// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugSubsyteme.h"


void UDebugSubsyteme::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("--- DebugSubsyteme Initialized ---"));
	static TSharedPtr<FDebugWindow> GDebugWindow;

	if (!GDebugWindow.IsValid())
	{
		GDebugWindow = MakeShared<FDebugWindow>();
	}

	DebugWindow = GDebugWindow;
	
}
void UDebugSubsyteme::Deinitialize()
{


	Super::Deinitialize();
}