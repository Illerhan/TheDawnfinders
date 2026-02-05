#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "CurseStatue.generated.h"

UCLASS()
class THEDAWNFINDERS_API ACurseStatue : public ATrapBase
{
	GENERATED_BODY()
	
public:
	ACurseStatue();
	virtual void DoTrapAction() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OpenStatueEyes(float Duration);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CloseStatueEyes();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeployCurse();


protected :
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Statue")
	float OpenedEyesDuration;

	UPROPERTY(BlueprintReadWrite, Category = "Statue")
	TArray<AActor*> PlayersInSight;

	UPROPERTY(BlueprintReadWrite, Category = "Statue")
	float EyesOpenedTimer;

	UPROPERTY(BlueprintReadWrite, Category = "Statue")
	bool CurseDeployed;
};
