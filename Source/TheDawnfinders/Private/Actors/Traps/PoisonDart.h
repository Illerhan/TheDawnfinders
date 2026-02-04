// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Activable.h"
#include "PoisonDart.generated.h"

class ADarts;

UCLASS()
class THEDAWNFINDERS_API APoisonDart : public AActor, public IActivable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APoisonDart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void DoMainAction_Implementation() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<ADarts> Dart;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UArrowComponent* Arrow;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Cooldown;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
	float CurrentCooldown;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
	bool bCooldown;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
