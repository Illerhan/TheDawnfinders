#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IFadeable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UFadeable : public UInterface 
{
	GENERATED_BODY()
};

class IFadeable 
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Fadeable")
	void FadeIn();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Fadeable")
	void FadeOut();
};