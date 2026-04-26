#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "ALandmine.generated.h"

UCLASS()
class THEDAWNFINDERS_API ALandmine : public ATrapBase
{
	GENERATED_BODY()
	
public :
	virtual void DoTrapAction(AActor* OtherActor) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Explose();

protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmine")
	float ExplosionRange;
};
