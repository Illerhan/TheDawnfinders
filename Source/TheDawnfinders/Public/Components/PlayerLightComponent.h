// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "PlayerLightComponent.generated.h"


UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class THEDAWNFINDERS_API UPlayerLightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerLightComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Light")
	USceneComponent* LightRoot;
		
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Lantern")
	float LightIntensity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float LightRadius;

	UPROPERTY(ReplicatedUsing = OnRep_LightOn, BlueprintReadWrite, EditAnywhere, Category="Lantern")
	bool bLightOn;

	UFUNCTION(BlueprintCallable, Category = "Light")
	void TurnLightOn();

	UFUNCTION(BlueprintCallable, Category = "Light")
	void TurnLightOff();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	bool bVivianiteUsed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float VivianiteDuration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float VivianiteTimeLeft;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	UStaticMeshComponent* LightMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	UPointLightComponent* PointLight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Lantern")
	USphereComponent* ProtectionZone;

	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable)
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_TurnLightOn();

	UFUNCTION(Server, Reliable)
	void Server_TurnLightOff();

	// Appelé quand bLightOn change (pour réplication)
	UFUNCTION()
	void OnRep_LightOn();

	// Applique visuellement l'état de la lumière
	void ApplyLightState();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
