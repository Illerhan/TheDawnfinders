#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CustomHUD.generated.h"

class UMainWidget;


UCLASS()
class THEDAWNFINDERS_API ACustomHUD : public AHUD
{
	GENERATED_BODY()

public :

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMainWidget* MainWidget;
};
