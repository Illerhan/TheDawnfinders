// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "UPlayerLightComponent.generated.h"


UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class THEDAWNFINDERS_API UPlayerLightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerLightComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


// === TURN ON / OFF ===
public :
	UFUNCTION(BlueprintCallable)
	void InitialiseComponent(float FuelConsumption, float MaxFuel);
	
	UFUNCTION(BlueprintCallable, Category = "Light")
	void TurnLightOn();

	UFUNCTION(BlueprintCallable, Category = "Light")
	void TurnLightOff();

	UFUNCTION(Server, Reliable)
	void Server_TurnLightOn();

	UFUNCTION(Server, Reliable)
	void Server_TurnLightOff();

	UFUNCTION()
	void ApplyLightState();

	UFUNCTION()
	void OnRep_LightOn();
	UFUNCTION()
	void FuelUpdate(float NewFuel);
	UFUNCTION()
	void OnRep_FuelRemaining();


	// === FUEL + PROTECTION ===
private :
	UFUNCTION()
	void ConsumeFuel(float DeltaTime);

	bool HasFuel() const
	{
		return FuelRemaining > 0.0f;
	}


public :
	UFUNCTION(Server,Unreliable)
	void Server_RequestFuelUpdate(float Amount);
	
	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


// === COMPONENTS ===
public :
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	UStaticMeshComponent* LightMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	UPointLightComponent* PointLight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	USphereComponent* ProtectionZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Light")
	USceneComponent* LightRoot;


// === PUBLIC PROPERTIES ===
public :
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Lantern")
	float LightIntensity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float LightRadius;

	UPROPERTY(ReplicatedUsing = OnRep_LightOn, BlueprintReadWrite, EditAnywhere, Category="Lantern")
	bool bLightOn;   

	UPROPERTY(Replicated, EditAnywhere, Category = "Lantern")
	float MaxFuel;
	
	UPROPERTY(ReplicatedUsing = OnRep_FuelRemaining,EditAnywhere, BlueprintReadWrite, Category = "Lantern")
	float FuelRemaining;

	UPROPERTY(Replicated,EditAnywhere, BlueprintReadWrite, Category = "Lantern")
	float FuelConsumption;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	bool bVivianiteUsed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float VivianiteDuration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float VivianiteTimeLeft;
};
