// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "UPlayerLightComponent.generated.h"

class UPalanquinHUDWidget;


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
	void InitialiseComponent(float FuelRate, float MaxFuels);
	
	UFUNCTION(BlueprintCallable, Category = "Light")
	void TurnLightOn();

	UFUNCTION(BlueprintCallable, Category = "Light")
	void TurnLightOff();

	UFUNCTION(Server, Reliable)
	void Server_TurnLightOn();

	UFUNCTION(Server, Reliable)
	void Server_TurnLightOff();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ApplyLightState();

	UFUNCTION()
	void OnRep_LightOn();

	UFUNCTION(BlueprintCallable)
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
	void UpdateProtectionZoneRadius();
	
	UFUNCTION(Server, Unreliable)
	void Server_RequestStorageUpdate(float VivianiteSize);
	
	UFUNCTION(BlueprintCallable)
	void StoreFuel(float VivianiteSize);

public :	
	UFUNCTION(Server,Unreliable)
	void Server_RequestFuelUpdate();
	
	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Collision")
	void OnFogOfWarOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Collision")
	void OnFogOfWarOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	

// === PUBLIC PROPERTIES ===
public :
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Lantern")
	float LightIntensity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float LightRadius;

	UPROPERTY(ReplicatedUsing = OnRep_LightOn, BlueprintReadWrite, EditAnywhere, Category="Lantern")
	bool bLightOn;   

	UPROPERTY(Replicated, EditAnywhere, Category = "Lantern")
	float MaxFuel = 1200;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float MidFuel = 800;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float LowFuel = 400;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float RefileValue = 400;
	
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float FuelStorage = 1200;
	
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	float MaxRadius;
	
	


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintReadOnly)
	UPalanquinHUDWidget* PalanquinHUDWidget;
};
