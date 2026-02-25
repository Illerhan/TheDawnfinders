#include "Components/UPlayerLightComponent.h"

#include <gsl/pointers>

#include "Kismet/GameplayStatics.h"
#include "Actors/Interactibles/Litter.h"
#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/CustomPlayerState.h"
#include "GameFramework/CustomHUD.h"
#include "Components/UHealthComponent.h"
#include "Widgets/UPalanquinHUD.h"
#include "Widgets/UMainWidget.h"
#include "Interfaces/IFadeable.h"
#include "Net/UnrealNetwork.h"


UPlayerLightComponent::UPlayerLightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	

	SetIsReplicatedByDefault(true);
}

void UPlayerLightComponent::BeginPlay()
{
	Super::BeginPlay();
    
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Variables locales pour stocker les composants à binder
	UPrimitiveComponent* ZoneToBind = nullptr;
	UPrimitiveComponent* FogOffToBind = nullptr;
	UPrimitiveComponent* FogOnToBind = nullptr;

	// On récupère les références selon la classe
	if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Owner))
	{
		ZoneToBind = Player->ProtectionZone;
		FogOffToBind = Player->FogOfWarLightOff;
		FogOnToBind = Player->FogOfWarLightOn;
	}
	else if (ALitter* Litter = Cast<ALitter>(Owner))
	{
		ZoneToBind = Litter->ProtectionZone;
		FogOffToBind = Litter->FogOfWarLightOff;
		FogOnToBind = Litter->FogOfWarLightOn;
	}
	
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (ZoneToBind)
		{
			ZoneToBind->OnComponentBeginOverlap.AddDynamic(this, &UPlayerLightComponent::OnOverlapBegin);
			ZoneToBind->OnComponentEndOverlap.AddDynamic(this, &UPlayerLightComponent::OnOverlapEnd);
		}
        
		if (FogOffToBind)
		{
			FogOffToBind->OnComponentBeginOverlap.AddDynamic(this, &UPlayerLightComponent::OnFogOfWarOverlapBegin);
			FogOffToBind->OnComponentEndOverlap.AddDynamic(this, &UPlayerLightComponent::OnFogOfWarOverlapEnd);
		}

		if (FogOnToBind)
		{
			FogOnToBind->OnComponentBeginOverlap.AddDynamic(this, &UPlayerLightComponent::OnFogOfWarOverlapBegin);
			FogOnToBind->OnComponentEndOverlap.AddDynamic(this, &UPlayerLightComponent::OnFogOfWarOverlapEnd);
		}
	}

	ApplyLightState();
	
	// We get the UI ref
	if (!Owner) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !PC->IsLocalController()) return;

	ACustomHUD* HUD = Cast<ACustomHUD>(PC->GetHUD());
	if (!HUD) return;

	UMainWidget* MainWidget = HUD->MainWidget;
	if (!MainWidget) return;

	PalanquinHUDWidget = MainWidget->GetPalanquinHUDWidget();
}


void UPlayerLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bLightOn)
	{
		ConsumeFuel(DeltaTime);
	}
}

void UPlayerLightComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerLightComponent, bLightOn);
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerLightComponent, FuelRemaining, COND_None, REPNOTIFY_Always);
}

void UPlayerLightComponent::InitialiseComponent(float FuelRate, float MaxFuels)
{
	FuelConsumption= FuelRate;
	MaxFuel = MaxFuels;
	FuelRemaining = MaxFuel;
}


#pragma region Colliders

void UPlayerLightComponent::OnFogOfWarOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->Implements<UFadeable>()) return;

	OtherActor->SetOwner(GetOwner());
	IFadeable::Execute_FadeIn(OtherActor);
}

void UPlayerLightComponent::OnFogOfWarOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor->Implements<UFadeable>()) return;
	IFadeable::Execute_FadeOut(OtherActor);
}

#pragma endregion


#pragma region Turn On / Off

void UPlayerLightComponent::TurnLightOn()
{
	if (bLightOn) return;

	// Server
	if (GetOwner()->HasAuthority())
	{
		bLightOn = true;
		ApplyLightState();
	}
	else   // Client
	{
		Server_TurnLightOn();
	}
}

void UPlayerLightComponent::TurnLightOff()
{
	if (!bLightOn) return;
	
	if (GetOwner()->HasAuthority())
	{
		bLightOn = false;
		ApplyLightState();
	}
	else
	{
		Server_TurnLightOff();
	}
}


void UPlayerLightComponent::Server_TurnLightOn_Implementation()
{
	TurnLightOn();
}

void UPlayerLightComponent::Server_TurnLightOff_Implementation()
{
	TurnLightOff();
}


void UPlayerLightComponent::ApplyLightState_Implementation()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;
	// 1. DÉCLARATION : On prépare des pointeurs vides pour les composants dont on a besoin
	UPointLightComponent* TargetLight = nullptr;
	UPrimitiveComponent* TargetProtectionZone = nullptr;

	// 2. SELECTION : On remplit ces pointeurs selon le type de l'Owner
	if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Owner))
	{
		TargetLight = Player->PointLight;
		TargetProtectionZone = Player->ProtectionZone;
	}
	else if (ALitter* Litter = Cast<ALitter>(Owner))
	{
		// Je suppose ici que ALitter possède aussi ces membres exacts
		TargetLight = Litter->PointLight;
		TargetProtectionZone = Litter->ProtectionZone;
	}
	
	if (!TargetLight || !TargetProtectionZone) return;

	TargetLight->SetVisibility(bLightOn);
    
	TargetProtectionZone->SetHiddenInGame(!bLightOn);
	TargetProtectionZone->SetGenerateOverlapEvents(bLightOn);

	if (!Owner->HasAuthority()) return; 

	TArray<AActor*> OverlappingActors;
	TargetProtectionZone->GetOverlappingActors(OverlappingActors, AAPlayerCharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == Owner) continue;

		if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Actor))
		{
			if (bLightOn) Player->HealthComponent->AddProtectionZone();
			else Player->HealthComponent->RemoveProtectionZone();
		}
	}
}

void UPlayerLightComponent::OnRep_LightOn()
{
	ApplyLightState();
}

#pragma endregion


#pragma region Fuel + Protection

void UPlayerLightComponent::ConsumeFuel(float DeltaTime)
{
	ALitter* Litter = Cast<ALitter>(GetOwner());
	if (!Litter) return;
	// Server Changes values
	if (GetOwner()->HasAuthority()) 
	{
		FuelRemaining -= FuelConsumption * DeltaTime;
		FuelRemaining = FMath::Max(FuelRemaining, 0.f);
		
		
		if (FuelRemaining > MidFuel)
		{
			Litter->ProtectionZone->SetSphereRadius(MaxRadius);
			Litter->PointLight->SetIntensity(LightRadius);
			
		}
		if (FuelRemaining < MidFuel && FuelRemaining> LowFuel)
		{
			Litter->ProtectionZone->SetSphereRadius(MidFuel/MaxFuel * MaxRadius);
			Litter->PointLight->SetIntensity(MidFuel/MaxFuel *LightRadius);
		}
		if (FuelRemaining < LowFuel)
		{
			Litter->ProtectionZone->SetSphereRadius(LowFuel/MaxFuel * MaxRadius);
			Litter->PointLight->SetIntensity(LowFuel/MaxFuel *LightRadius);
		}
		
		if (Litter->PointLight)
		{
			Litter->PointLight->MarkRenderStateDirty();
		}
			
		
			
		if (FuelRemaining <= 0.f && bLightOn)
		{
			TurnLightOff();
		}
		
		UpdateProtectionZoneRadius();
	}

	if (!PalanquinHUDWidget)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (!PC || !PC->IsLocalController()) return;

		ACustomHUD* HUD = Cast<ACustomHUD>(PC->GetHUD());
		if (!HUD) return;

		UMainWidget* MainWidget = HUD->MainWidget;
		if (!MainWidget) return;

		PalanquinHUDWidget = MainWidget->GetPalanquinHUDWidget();
	}

	PalanquinHUDWidget->ActualiseWidget(FuelRemaining / MaxFuel);
}

void UPlayerLightComponent::UpdateProtectionZoneRadius()
{
	float FuelValue = 0;
	if (FuelRemaining > MidFuel) FuelValue = MaxFuel;
	if (FuelRemaining < MidFuel && FuelRemaining> LowFuel) FuelValue = MidFuel;
	if (FuelRemaining < LowFuel) FuelValue = LowFuel;
	
	float FuelAlpha = (MaxFuel > 0.f) ? (FuelValue / MaxFuel) : 0.f;
	
	float TargetRadius = FuelAlpha * MaxRadius; 
	
	AActor* Owner = GetOwner();
    
	if (ALitter* Litter = Cast<ALitter>(Owner))
	{
		if (Litter->ProtectionZone)
		{
			
			Litter->ProtectionZone->SetSphereRadius(TargetRadius);

			if (Litter->PointLight) Litter->PointLight->SetAttenuationRadius(TargetRadius);
			Litter->PointLight->MarkRenderStateDirty();
		}
	}
	else if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Owner))
	{
		if (Player->ProtectionZone)
		{
			Player->ProtectionZone->SetSphereRadius(TargetRadius);
		}
	}
}

void UPlayerLightComponent::Server_RequestStorageUpdate_Implementation(float VivianiteSize)
{
	StoreFuel(VivianiteSize);
}


void UPlayerLightComponent::StoreFuel(float VivianiteSize)
{
	if (GetOwner()->HasAuthority())
	{
		FuelStorage += VivianiteSize;
		FuelStorage = FMath::Clamp(FuelStorage, 0.f, MaxFuel);
	}
}

void UPlayerLightComponent::FuelUpdate()
{
	if (GetOwner()->HasAuthority())
	{
		if (FuelStorage < RefileValue) return;
		FuelRemaining += RefileValue;
		FuelStorage-= RefileValue;
		FuelRemaining = FMath::Clamp(FuelRemaining, 0.f, MaxFuel);

		if (FuelRemaining <= 0.f && bLightOn)
		{
			TurnLightOff();
		}
		else if (!bLightOn && FuelRemaining > 0.f)
		{
			TurnLightOn();
		}
	}

	if (!PalanquinHUDWidget)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (!PC || !PC->IsLocalController()) return;

		ACustomHUD* HUD = Cast<ACustomHUD>(PC->GetHUD());
		if (!HUD) return;

		UMainWidget* MainWidget = HUD->MainWidget;
		if (!MainWidget) return;

		PalanquinHUDWidget = MainWidget->GetPalanquinHUDWidget();
	}

	PalanquinHUDWidget->ActualiseWidget(FuelRemaining / MaxFuel);
}

void UPlayerLightComponent::OnRep_FuelRemaining()
{
	
	UpdateProtectionZoneRadius();
	
	if (!PalanquinHUDWidget)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (!PC || !PC->IsLocalController()) return;

		ACustomHUD* HUD = Cast<ACustomHUD>(PC->GetHUD());
		if (!HUD) return;

		UMainWidget* MainWidget = HUD->MainWidget;
		if (!MainWidget) return;

		PalanquinHUDWidget = MainWidget->GetPalanquinHUDWidget();
	}

	PalanquinHUDWidget->ActualiseWidget(FuelRemaining / MaxFuel);
}



void UPlayerLightComponent::Server_RequestFuelUpdate_Implementation()
{
	FuelUpdate();
}

void UPlayerLightComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetOwner()->HasAuthority() || !bLightOn) return;
	if (OtherActor == GetOwner()) return;

	if (OtherActor->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(OtherActor);

		PlayerInterface->AddProtectionZone_Implementation();
	}
}

void UPlayerLightComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetOwner()->HasAuthority() || !bLightOn) return;
	if (OtherActor == GetOwner()) return;

	if (OtherActor->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(OtherActor);

		PlayerInterface->RemoveProtectionZone_Implementation();
	}
}

#pragma endregion

