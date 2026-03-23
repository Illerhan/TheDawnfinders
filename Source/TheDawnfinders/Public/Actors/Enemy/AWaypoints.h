#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AWaypoints.generated.h"

UCLASS()
class THEDAWNFINDERS_API AWaypoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaypoint();
	virtual void BeginPlay() override;
};
