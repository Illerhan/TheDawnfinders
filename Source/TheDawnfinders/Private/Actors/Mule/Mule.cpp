// Fill out your copyright notice in the Description page of Project Settings.


#include "Mule.h"

#include "../../../../../Plugins/WwiseSoundEngine/ThirdParty/include/AK/WwiseAuthoringAPI/waapi.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/UWorldInteractibleWidget.h"


// Sets default values
AMule::AMule()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	LootCollider = CreateDefaultSubobject<UBoxComponent>(FName("LootCollider"));
	LootCollider->SetupAttachment(RootComponent);
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(FName("InventoryComponent"));
	InteractibleWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(FName("InteractibleWidgetComponent"));
	InteractibleWidgetComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMule::BeginPlay()
{
	InteractibleWidget = Cast<UWorldInteractibleWidget>(InteractibleWidgetComponent->GetWidget());

	Super::BeginPlay();
	
}

// Called every frame
void AMule::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMule::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMule::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMule, ChargeCooldown);
	DOREPLIFETIME(AMule, CallCooldown);
	DOREPLIFETIME(AMule, MaxCharges);
	DOREPLIFETIME(AMule, CallCharges);
	DOREPLIFETIME(AMule, CooldownTimer);
	DOREPLIFETIME(AMule, ChargesTimer);
}
