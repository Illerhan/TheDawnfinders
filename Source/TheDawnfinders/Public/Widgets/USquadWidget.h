#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/USquadMemberWidget.h"
#include "USquadWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API USquadWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;


// === MAIN FUNCTIONS ===
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseSquadInfos();

	UFUNCTION(BlueprintCallable)
	void BindAllCurrentPlayerStates();

	UFUNCTION(BlueprintCallable)
	void BindNewPlayerState(APlayerState* State);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddNewSquadMember();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void RemoveSquadMember();
	

// === PROTECTED PROPERTIES ===
protected:
	UPROPERTY()
	bool bWidgetsInitialized = false;

	UPROPERTY()
	bool bAlreadyBound = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxSquadMembers = 4;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<USquadMemberWidget*> SquadMemberWidgets;

	FTimerHandle BindDelayTimerHandle;
};
