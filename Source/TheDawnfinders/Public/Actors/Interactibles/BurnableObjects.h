// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/UPlayerLightComponent.h"
#include "Actors/Interactibles/Interactible.h"
#include "Net/UnrealNetwork.h"
#include "BurnableObjects.generated.h"

UCLASS()
class THEDAWNFINDERS_API ABurnableObjects : public AInteractibleObjects
{
    GENERATED_BODY()

public:
    ABurnableObjects();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public :
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light")
    UPlayerLightComponent* LightComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burning", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Burning")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burning", meta = (ClampMin = "0.1"))
    float BurnRate = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Burning")
    void LightObject();

    UFUNCTION(BlueprintCallable, Category = "Burning")
    void ExtinguishObject();

    UFUNCTION(BlueprintCallable, Category = "Burning")
    void AddHealth(float Amount);

    UFUNCTION(BlueprintPure, Category = "Burning")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Burning")
    float GetEstimatedTimeRemaining() const;

    UFUNCTION()
    void OnRep_CurrentHealth();

private:
    UPROPERTY(ReplicatedUsing = OnRep_IsBurning)
    bool bIsBurning;

    UFUNCTION()
    void OnRep_IsBurning();

    // Function 
    void OnHealthDepleted();

    // Consommer la vie de l'objet
    void ConsumeHealth(float DeltaTime);
};