#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "USquadMemberWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API USquadMemberWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Initialise(bool IsMainSlot);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualisePlayersFace(int Index, const FString& PlayerName);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseWidget(float currentHealth, float currentMaxHealth, float currentStamina, float maxStamina, float maxHealth, FInventorySlot CurrentItem);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetPoisoned(bool bPoisoned);

protected :
	UPROPERTY(BlueprintReadWrite)
	bool bIsPoisoned;
};
