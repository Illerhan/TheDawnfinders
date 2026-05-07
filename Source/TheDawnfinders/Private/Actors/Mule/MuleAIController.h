#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AkAudioEvent.h"
#include "Actors/Player/ANoise.h"
#include "Actors/Mule/Mule.h"
#include "MuleAIController.generated.h"

UCLASS()
class THEDAWNFINDERS_API AMuleAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMuleAIController();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = Mule)
	void CallMule(AActor* Actor);
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multi_PlaySound(FVector Position = FVector::ZeroVector);
	
	FTimerHandle TimerHandle_SpawnObject;
	
	UFUNCTION(BlueprintCallable, Category = Mule)
	void OnSpawnTimerExpired(FVector SpawnPos);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mule)
	float SpawnOffset;

	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category = Mule)
	float SpawnDelay;
	
		
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAkAudioEvent* MuleTravelSound;
	
	UPROPERTY()
	int32 MuleTravelSoundID;


protected :
	UPROPERTY(BlueprintReadWrite)
	AMule* MyMule;
};