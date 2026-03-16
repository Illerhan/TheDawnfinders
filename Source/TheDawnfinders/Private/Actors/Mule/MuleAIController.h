#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
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
};