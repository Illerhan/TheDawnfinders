// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/CustomPlayerState.h"
#include "GameFramework/CustomHUD.h"
#include "Widgets/UMainWidget.h"
#include "Widgets/USquadWidget.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/UStaminaComponent.h"
#include "Net/UnrealNetwork.h"

void ACustomPlayerState::BeginPlay()
{
	bReplicates = true;

	APlayerController* PC = GetPlayerController();
	if (!PC) return;
	
	ACustomHUD* HUD = Cast<ACustomHUD>(PC->GetHUD());
	if (!HUD) return;

	UMainWidget* MainWidget = Cast<UMainWidget>(HUD->MainWidget);
	if (!MainWidget) return;

	//GetWorld()->GetGameState()->PlayerArray;



	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return;
	
	AAPlayerCharacter* PlayerCharacter = Cast<AAPlayerCharacter>(PlayerPawn);
	if (!PlayerCharacter || !PlayerCharacter->StaminaComponent) return;
	
	//PlayerCharacter->StaminaComponent->OnStaminaChange.AddUniqueDynamic(this, &ACustomPlayerState::ActualiseStamina);
}

//void ACustomPlayerState::GetRepLifetimeReplicatedProps()

void ACustomPlayerState::ActualiseStamina(float current, float max)
{
	CurrentStamina = current;
	CurrentMaxStamina = max;

	OnRep_StaminaChange();
}

void ACustomPlayerState::ActualiseHealth(float current, float max)
{
	CurrentHealth = current;
	CurrentMaxHealth = max;

	OnRep_StaminaChange();
}

void ACustomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACustomPlayerState, CurrentStamina);
}

void ACustomPlayerState::OnRep_StaminaChange()
{
	OnInfoChange.Broadcast();
}
