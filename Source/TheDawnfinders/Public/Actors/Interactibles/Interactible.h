// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractible.h"
#include "Interfaces/IFadeable.h"
#include "Components/TimelineComponent.h" 
#include "Curves/CurveFloat.h"
#include "Interactible.generated.h"

class UBoxComponent;
class AAPlayerCharacter;
class ULockpickQTEWidget;
class UWorldInteractibleWidget;


UCLASS()
class THEDAWNFINDERS_API AInteractibleObjects : public AActor, public IInteractible, public IFadeable
{
	GENERATED_BODY()

public:
	AInteractibleObjects();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	virtual void HoldTimer(float DeltaTime);


// === INTERACTION INTERFACE ===
public :
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;
	virtual bool GetCanBeUsed_Implementation() override;
	virtual bool GetQTENeeded_Implementation() override;
	virtual void StartQTE_Implementation() override;
	virtual void StopQTE_Implementation() override;
	virtual bool ValidateQTE_Implementation() override;


// === MAIN FUNCTIONS ===
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void BP_OnInteraction(AAPlayerCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void BP_OnStopInteraction(AAPlayerCharacter* Player);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void BP_OnInteractionFinished();


// === FADEABLE ===
public :
	virtual void FadeIn_Implementation() override;
	virtual void FadeOut_Implementation() override;

	UFUNCTION(NetMulticast, Reliable)
	void FadeIn_Multicast();

	UFUNCTION(NetMulticast, Reliable)
	void FadeOut_Multicast();

	UFUNCTION()
	void HandleFadeProgress(float Value);


// === OTHERS ===
public:
	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DisplayErrorMessage(const FString& Message);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_DisplayErrorMessage(const FString& Message);


// === COMPONENTS ===
public :
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Collision")
	UBoxComponent* BoxCollider;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Mesh")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Widgets")
	UWidgetComponent* InteractQTEWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Widgets")
	UWidgetComponent* InteractibleWidgetComponent;


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDoQTE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QTESuccessRangeStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QTESuccessRangeEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int QTEStepsCount;

	UPROPERTY()
	bool bCanBeUsed = true;

	UPROPERTY()
	ULockpickQTEWidget* InteractQTEWidget;

	UPROPERTY()
	UWorldInteractibleWidget* InteractibleWidget;

	UPROPERTY(Replicated)
	float InteractionTimer;

	UPROPERTY(Replicated)
	bool bIsInteracting;

	UPROPERTY(Replicated, BlueprintReadWrite)
	AActor* PlayerTemp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* ChestSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SoundLoudness;

	UPROPERTY()
	FTimeline FadeTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* FloatCurve;

	UPROPERTY(BlueprintReadWrite)
	int FogOfWarCount = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInstanceDynamic*> Materials;
};
