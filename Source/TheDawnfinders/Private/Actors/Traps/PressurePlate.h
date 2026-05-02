// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AkAudioEvent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "PressurePlate.generated.h"

UENUM(BlueprintType)
enum class EPlateActivationMode : uint8
{
	Hold        UMETA(DisplayName = "Maintenir"), // Actif tant qu'on reste dessus
	Toggle      UMETA(DisplayName = "Bascule"),  // Change d'état à chaque pression
	Once        UMETA(DisplayName = "Une seule fois") // S'active et reste actif
};

UCLASS()
class THEDAWNFINDERS_API APressurePlate : public AActor
{
	GENERATED_BODY()

public:
	APressurePlate();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditAnywhere, Category = "Plate Settings")
	EPlateActivationMode ActivationMode = EPlateActivationMode::Hold;


// === Functions ===
public :
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void ExecutePlateAction(bool bActivate);

	UFUNCTION(Netmulticast, Unreliable)
	void Multi_PlaySound();

	UPROPERTY(Blueprintable, EditAnywhere)
	UBoxComponent* BoxCollider;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsActive,BlueprintReadWrite, EditAnywhere)
	bool bIsActive;
	
	UFUNCTION(BlueprintCallable,Blueprintable,BlueprintImplementableEvent)
	void OnRep_IsActive();


// === Properties ====
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int NeededPlayerCount = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bActivatesWithPalanquin = false;

	UPROPERTY(Blueprintable ,EditAnywhere)
	TArray<AActor*> LinkedActors;

	UPROPERTY()
	int CurrentPlayerCount = 0;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAkAudioEvent* PlateSound;
	
	UPROPERTY()
	int32 PlateSoundID;
	
};
