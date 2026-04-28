// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssetList.h"
#include "Engine/DataAsset.h"
#include "ShopDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class THEDAWNFINDERS_API UShopDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<FName, FDataAssetList> CategoriesMap;
};
