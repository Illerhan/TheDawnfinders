// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CustomStructs.h"
#include "DataAssets/ItemData.h"
#include "UInventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(BlueprintReadWrite)
	int CurrentSlotIndex;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FInventorySlot GetCurrentSlot();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FInventorySlot ChangeCurrentSlot(bool IndexGoUp);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddNewItem(UItemData* NewItem);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Throw();
};
