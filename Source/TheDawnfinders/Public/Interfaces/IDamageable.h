#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IDamageable.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};


class IDamageable
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damageable")
	void ReceiveDamage(float Quantity, AActor* Origin);
};
