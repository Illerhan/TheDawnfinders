#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAssetList.generated.h"

USTRUCT(BlueprintType)
struct FDataAssetList
{
	GENERATED_BODY()

public:
	// Le tableau de Data Assets (remplace UDataAsset par ta classe spécifique si besoin)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TArray<UDataAsset*> Items;
};
