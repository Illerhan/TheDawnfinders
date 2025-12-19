#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CustomGameMode.generated.h"


UCLASS()
class THEDAWNFINDERS_API ACustomGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
};
