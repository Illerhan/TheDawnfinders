#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/UWorldHealthBar.h"
#include "UEnemyWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API UEnemyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_PlayAggressiveAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayAggressiveAnim();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_PlaySuspiciousAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlaySuspiciousAnim();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_PlayListeningAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayListeningAnim();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_StopListeningAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StopListeningAnim();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_HideAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HideAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UWorldHealthBar* GetHealthBarWidget();
};
