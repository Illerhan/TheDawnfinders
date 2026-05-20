#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GICustom.generated.h"


UCLASS()
class THEDAWNFINDERS_API UGICustom : public UGameInstance
{
	GENERATED_BODY()
	
public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FColor> ColorPerRarity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequestedPanel = 1;

};
