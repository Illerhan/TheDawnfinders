// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugWindow.h"

#include "Components/UHealthComponent.h"
#include "Components/UItemComponent.h"
#include "Kismet/GameplayStatics.h"       // Pour trouver le PlayerPawn
#include "Editor/EditorEngine.h"          // Pour accéder à GEditor
#include "Public/Actors/Player/APlayerCharacter.h"
#include "Private/GameFramework/SessionManagerSubsystem.h"

FDebugWindow::FDebugWindow()
	: FSlateIMWindowBase(TEXT("Ma Fenêtre Debug"), FVector2f(400, 600), TEXT("MyDebug.Toggle"), TEXT("Ouvre ma fenêtre de debug"))
{
	UE_LOG(LogTemp, Warning, TEXT("--- FDebugWindow CONSTRUCTOR CALLED ---"));
}

void FDebugWindow::DrawWindow(float DeltaTime)
{
	UWorld* GameWorld = GEditor ? GEditor->PlayWorld : nullptr;
	if (!GameWorld)
	{
		SlateIM::Text(TEXT("Le jeu n'est pas lancé. En attente..."), FLinearColor::Gray);
		return; // On arrête là, pas de joueur à chercher
	}
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GameWorld, 0);

	AAPlayerCharacter* PlayerChar = Cast<AAPlayerCharacter>(PlayerPawn);

	if (!PlayerChar)
		return;
	UGameInstance* GameIntance = GameWorld->GetGameInstance();
	USessionManagerSubsystem* SessionManager = GameIntance->GetSubsystem<USessionManagerSubsystem>();;
	SlateIM::BeginVerticalStack();
	FString HealthText = FString::Printf(
		TEXT("Health : %.1f/%.1f"),
		PlayerChar->HealthComponent->CurrentHealth,
		PlayerChar->HealthComponent->MaxHealth);
	SlateIM::SpinBox(PlayerChar->HealthComponent->CurrentHealth,
		0.0f,
		PlayerChar->HealthComponent->MaxHealth);

	if (SlateIM::Button(TEXT("Revive")))
	{
		PlayerChar->HealthComponent->Server_Revive();
		PlayerChar->HealthComponent->CurrentHealth = PlayerChar->HealthComponent->MaxHealth;
	}
	SlateIM::SpinBox(PlayerChar->PlayerConfig->RunSpeed,0.f,1000.f);
	
	if (SlateIM::Button(TEXT("CreateSession")))
	{
		SessionManager->CreateAdvancesSession(
		4,
		4,
		false,  
		true, 
		false,
		true    
	);
		UGameplayStatics::OpenLevel(GameWorld, FName("Lvl_TopDown"),false,"listen?");
	}
	if (SlateIM::Button(TEXT("Start")))
	{
		GameWorld->ServerTravel("L_Prototype?listen",true);
	}
	SlateIM::EndVerticalStack();
}