// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomPlayerController.h"

#include "Actors/Mule/DangerManager.h"
#include "Actors/Mule/Mule.h"
#include "Actors/Mule/MuleAIController.h"
#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/GICustom.h"
#include "Widgets/Mule/MuleWidget.h"

void ACustomPlayerController::Server_RequestSwitchPanel_Implementation(int32 PanelIndex)
{
	// Le serveur broadcast à tout le monde
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACustomPlayerController* PC = Cast<ACustomPlayerController>(It->Get());
		if (PC) PC->Multicast_SwitchPanel(PanelIndex);
	}
}

void ACustomPlayerController::Multicast_SwitchPanel_Implementation(int32 PanelIndex)
{
	if (!IsLocalController()) return;
	SwitchPanelBP(PanelIndex);
}

void ACustomPlayerController::Server_CallMule_Implementation(AActor* Actor)
{
	AMuleAIController* MuleAIController = Cast<AMuleAIController>(Mule->GetController());
	if (MuleAIController)
	{
		if (Mule->CooldownTimer > 0.f) return;
		MuleAIController->CallMule(Actor);
		Mule->DangerManager->MuleCalled();
	}
}

void ACustomPlayerController::Client_SetRequestedPanel_Implementation(int32 PanelIndex)
{
	UGICustom* GI = (Cast<UGICustom>(GetGameInstance()));
	if (GI)
	{
		GI->RequestedPanel = PanelIndex;
	}
}

void ACustomPlayerController::Server_HoldMule_Implementation(float DeltaTime)
{
	AAPlayerCharacter* OwningPlayer = Cast<AAPlayerCharacter>(GetCharacter());
	if (!OwningPlayer) return;
	
	AMuleAIController* MuleAIController = Cast<AMuleAIController>(Mule->GetController());
	if (MuleAIController) if (Mule->CooldownTimer > 0.f) return;
	
	HoldTimer -= DeltaTime;
	
	if (!NoiseActor)
	{
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		Params.Instigator = Cast<APawn>(GetOwner());
		
		ANoise* Noise = GetWorld()->SpawnActor<ANoise>(NoiseObject, OwningPlayer->GetActorLocation(), FRotator(0, 0, 0), Params);
		Noise->Radius = NoiseRange;
		Noise->bIsLoud = bIsLoud;
		Noise->NoiseOriginActor = OwningPlayer;
		Noise->SetIsConstant(true);
		NoiseActor = Noise;
	}
	NoiseActor->SetActorLocation(OwningPlayer->GetActorLocation());
		
	OwningPlayer->Execute_ShowProgress(OwningPlayer,HoldTimer);
	if (HoldTimer <= 0.f)
	{
		Server_CallMule(OwningPlayer);
		Server_StopHoldMule();
	}
		
}

void ACustomPlayerController::Server_StopHoldMule_Implementation()
{
	HoldTimer = InputHoldDuration;
	AAPlayerCharacter* OwningPlayer = Cast<AAPlayerCharacter>(GetCharacter());
	if (NoiseActor)
	{
		NoiseActor->Destroy();
		NoiseActor = nullptr;
	}
	
	if (!OwningPlayer) return;
	OwningPlayer->Execute_HideProgress(OwningPlayer);
	bIsHolding = false;
}

void ACustomPlayerController::RespawnToCheckpoint()
{
	
	ACharacter* OwnerChar = Cast<ACharacter>(GetPawn());
	if (!OwnerChar) return;
	
	OwnerChar->TeleportTo(SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation());
}

void ACustomPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocalController() || !MuleWidget || !Mule) return;
	float ChargesPercent = 0.f;
	if (Mule->ChargeCooldown > 0.f)
	{
		ChargesPercent = Mule->ChargesTimer / Mule->ChargeCooldown;
	}

	MuleWidget->UpdateMuleWidget(
		Mule->CallCharges,
		ChargesPercent,
		Mule->CooldownTimer,
		Mule->InventoryComponent->CurrentValue
	);
	
	if (bIsHolding)
	{
		Server_HoldMule(DeltaTime);
	}
}

void ACustomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	HoldTimer= InputHoldDuration;
}
