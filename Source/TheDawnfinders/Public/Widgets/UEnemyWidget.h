#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UEnemyWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API UEnemyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_PlaySuspiciousAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlaySuspiciousAnim();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_PlayAggressiveAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayAggressiveAnim();
};
