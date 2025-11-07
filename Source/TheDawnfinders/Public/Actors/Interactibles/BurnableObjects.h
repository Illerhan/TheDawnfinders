// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PlayerLightComponent.h"
#include "Actors/Interactibles/Interactible.h"
#include "Net/UnrealNetwork.h"
#include "BurnableObjects.generated.h"

UCLASS()
class THEDAWNFINDERS_API ABurnableObjects : public AInteractibleObjects
{
    GENERATED_BODY()

public:
    ABurnableObjects();

    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light")
    UPlayerLightComponent* LightComponent;

    // Vie maximale de l'objet (en points de vie)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burning", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.0f;

    // Vie actuelle de l'objet
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Burning")
    float CurrentHealth;

    // Vitesse de consommation de la vie par seconde quand l'objet brûle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burning", meta = (ClampMin = "0.1"))
    float BurnRate = 1.0f;

    // Fonction pour allumer l'objet
    UFUNCTION(BlueprintCallable, Category = "Burning")
    void LightObject();

    // Fonction pour éteindre l'objet
    UFUNCTION(BlueprintCallable, Category = "Burning")
    void ExtinguishObject();

    // Fonction pour ajouter de la vie (recharger l'objet)
    UFUNCTION(BlueprintCallable, Category = "Burning")
    void AddHealth(float Amount);

    // Obtenir le pourcentage de vie restante (0.0 à 1.0)
    UFUNCTION(BlueprintPure, Category = "Burning")
    float GetHealthPercentage() const;

    // Obtenir le temps restant estimé en secondes
    UFUNCTION(BlueprintPure, Category = "Burning")
    float GetEstimatedTimeRemaining() const;

protected:
    virtual void BeginPlay() override;

    // Fonction appelée quand CurrentHealth change (pour la réplication)
    UFUNCTION()
    void OnRep_CurrentHealth();

private:
    // Est-ce que l'objet brûle actuellement
    UPROPERTY(ReplicatedUsing = OnRep_IsBurning)
    bool bIsBurning;

    UFUNCTION()
    void OnRep_IsBurning();

    // Fonction appelée quand la vie atteint 0
    void OnHealthDepleted();

    // Consommer la vie de l'objet
    void ConsumeHealth(float DeltaTime);
};