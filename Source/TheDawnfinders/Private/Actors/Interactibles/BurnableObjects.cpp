// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactibles/BurnableObjects.h"
#include "Net/UnrealNetwork.h"

ABurnableObjects::ABurnableObjects()
{
    PrimaryActorTick.bCanEverTick = true;
    
    LightComponent = CreateDefaultSubobject<UPlayerLightComponent>(FName("AC_Light"));
    
    if (LightComponent && LightComponent->ProtectionZone)
    {
        LightComponent->ProtectionZone->SetupAttachment(RootComponent);
    }

    bIsBurning = false;
    CurrentHealth = MaxHealth;
    
    bReplicates = true;
}

void ABurnableObjects::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ABurnableObjects, CurrentHealth);
    DOREPLIFETIME(ABurnableObjects, bIsBurning);
}

void ABurnableObjects::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
}

void ABurnableObjects::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Si l'objet brûle, consommer la vie
    if (bIsBurning && HasAuthority())
    {
        ConsumeHealth(DeltaTime);
        
    }
}

void ABurnableObjects::ConsumeHealth(float DeltaTime)
{
    if (CurrentHealth > 0.0f)
    {
        CurrentHealth -= BurnRate * DeltaTime;
        CurrentHealth = FMath::Max(0.0f, CurrentHealth);

        // Optionnel : ajuster l'intensité de la lumière selon la vie restante
        if (LightComponent && LightComponent->PointLight)
        {
            float HealthPercentage = GetHealthPercentage();
            // Vous pouvez ajuster l'intensité ou d'autres propriétés ici
            // Par exemple : réduire l'intensité quand la vie diminue
            // LightComponent->PointLight->SetIntensity(BaseIntensity * HealthPercentage);
        }

        // Si la vie atteint 0
        if (CurrentHealth <= 0.0f)
        {
            OnHealthDepleted();
        }
    }
}

void ABurnableObjects::LightObject()
{
    if (!bIsBurning && HasAuthority())
    {
        // Vérifier qu'il reste de la vie
        if (CurrentHealth <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("%s cannot be lit - no health remaining"), *GetName());
            return;
        }

        bIsBurning = true;
        
        // Allumer la lumière
        if (LightComponent)
        {
            LightComponent->bLightOn = true;
            if (LightComponent->PointLight)
            {
                LightComponent->PointLight->SetVisibility(true);
                LightComponent->TurnLightOn();
            }
        }

        UE_LOG(LogTemp, Log, TEXT("%s is now burning (Health: %.1f/%.1f)"), 
               *GetName(), CurrentHealth, MaxHealth);
    }
}

void ABurnableObjects::ExtinguishObject()
{
    if (bIsBurning && HasAuthority())
    {
        bIsBurning = false;
        
        // Éteindre la lumière
        if (LightComponent)
        {
            LightComponent->bLightOn = false;
            if (LightComponent->PointLight)
            {
                LightComponent->PointLight->SetVisibility(false);
            }
        }

    }
}

void ABurnableObjects::AddHealth(float Amount)
{
    if (HasAuthority())
    {
        CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
        UE_LOG(LogTemp, Log, TEXT("%s health restored to %.1f/%.1f"), 
               *GetName(), CurrentHealth, MaxHealth);
    }
}

float ABurnableObjects::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ABurnableObjects::GetEstimatedTimeRemaining() const
{
    if (!bIsBurning || BurnRate <= 0.0f)
    {
        return 0.0f;
    }
    
    return CurrentHealth / BurnRate;
}

void ABurnableObjects::OnHealthDepleted()
{
    if (HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s health depleted - destroying object"), *GetName());
        
        // Éteindre la lumière
        ExtinguishObject();
        
        // Optionnel : ajouter des effets visuels/sonores avant destruction
        
        // Détruire l'acteur
        Destroy();
    }
}

void ABurnableObjects::OnRep_CurrentHealth()
{
    
    // Mettre à jour les effets visuels selon la vie
    if (LightComponent && LightComponent->PointLight)
    {
        float HealthPercentage = GetHealthPercentage();
        // Exemple : changer la couleur ou l'intensité
        // LightComponent->PointLight->SetIntensity(BaseIntensity * HealthPercentage);
    }
}

void ABurnableObjects::OnRep_IsBurning()
{
    // Appelé sur les clients quand bIsBurning change
    if (LightComponent)
    {
        LightComponent->bLightOn = bIsBurning;
        if (LightComponent->PointLight)
        {
            LightComponent->PointLight->SetVisibility(bIsBurning);
            LightComponent->TurnLightOn();
        }
    }
}