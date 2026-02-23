// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TimelineComponent.h" 
#include "Actors/Player/ANoise.h" 
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractible.h"
#include "Interfaces/IFadeable.h"
#include "Curves/CurveFloat.h"
#include "Interactible.generated.h"

class UBoxComponent;
class UItemData;
class AAPlayerCharacter;
class ULockpickQTEWidget;
class UWorldInteractibleWidget;


UENUM(BlueprintType)
enum class EInteractItemConsuptionType : uint8
{
	ConsumeOnQTEInput,
	ConsumeOnQTEFail,
	ConsumeOnUse
};


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

	UFUNCTION(BlueprintCallable)
	void PlayNoise();


// === INTERACTION INTERFACE ===
public :
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;
	virtual bool GetCanBeUsed_Implementation(AActor* Interactor) override;
	virtual EQTEType GetNeededQTE_Implementation() override;

	UFUNCTION(BlueprintCallable)
	virtual void OnQTESuccess();

	// Fonction pour gérer l'échec du QTE - à appeler depuis le widget
	UFUNCTION(BlueprintCallable)
	virtual void OnQTEFailed();

	UFUNCTION(BlueprintCallable)
	virtual void CheckEnableDistance();


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


// === GETTERS ===
public :
	UFUNCTION(BlueprintCallable)
	UItemData* GetNeededInteractItem() { return NeededInteractItem; }

	UFUNCTION(BlueprintCallable)
	EInteractItemConsuptionType GetInteractItemConsumptionType() { return InteractItemConsumptionType; }


// === COMPONENTS ===
public :
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Collision")
	UBoxComponent* BoxCollider;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Collision")
	UBoxComponent* InteractCollider;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Mesh")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Widgets")
	UWidgetComponent* InteractibleWidgetComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Widgets")
	UTexture2D* InputIcon;


// === QTE INFOS ===
public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	EQTEType QTEType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	float QTESuccessRangeStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	float QTESuccessRangeEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	int QTEStepsCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	float MashQTEQuantity = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	float MashQTEDecresePerSeconds = 2.f;


// === PROTECTED INFOS ===
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnableDistance = 3500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UItemData* NeededInteractItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInteractItemConsuptionType InteractItemConsumptionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	TSubclassOf<ANoise> NoiseActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float NoiseRange;

	UPROPERTY()
	FTimerHandle EnableTimer;

	UPROPERTY()
	bool bCanBeUsed = true;

	UPROPERTY()
	UWorldInteractibleWidget* InteractibleWidget;

	UPROPERTY(Replicated)
	float InteractionTimer;

	UPROPERTY(Replicated)
	bool bIsInteracting;

	UPROPERTY(Replicated, BlueprintReadWrite)
	AActor* PlayerTemp;

	UPROPERTY()
	FTimeline FadeTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* FloatCurve;

	UPROPERTY(BlueprintReadWrite)
	int FogOfWarCount = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInstanceDynamic*> Materials;
};
