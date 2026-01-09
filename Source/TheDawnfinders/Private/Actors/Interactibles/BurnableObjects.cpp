// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactibles/BurnableObjects.h"
#include "Net/UnrealNetwork.h"

ABurnableObjects::ABurnableObjects()
{
    PrimaryActorTick.bCanEverTick = true;
    
    //LightComponent = CreateDefaultSubobject<UPlayerLightComponent>(FName("AC_Light"));

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
        
        // Si la vie atteint 0
        if (CurrentHealth <= 0.0f)
        {
            OnHealthDepleted();
        }
    }
}

void ABurnableObjects::LightObject()
{
    if (!bIsBurning){
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
            LightComponent->ApplyLightState();
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
            // if (LightComponent->PointLight)
            // {
            //     LightComponent->PointLight->SetVisibility(false);
            // }
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
        UE_LOG(LogTemp, Warning, TEXT("%s health depleted - destroyinqg object"), *GetName());
        
        // Éteindre la lumière
        ExtinguishObject();
        
        // Détruire l'acteur
        Destroy();
    }
}

void ABurnableObjects::OnRep_CurrentHealth()
{
    
    // Mettre à jour les effets visuels selon la vie
    // if (LightComponent && LightComponent->PointLight)
    // {
    //     float HealthPercentage = GetHealthPercentage();
    //     // Exemple : changer la couleur ou l'intensité
    //     // LightComponent->PointLight->SetIntensity(BaseIntensity * HealthPercentage);
    // }
}

void ABurnableObjects::OnRep_IsBurning()
{
    // Appelé sur les clients quand bIsBurning change
    if (LightComponent)
    {
        LightComponent->bLightOn = bIsBurning;
        LightComponent->ApplyLightState();
    }
}