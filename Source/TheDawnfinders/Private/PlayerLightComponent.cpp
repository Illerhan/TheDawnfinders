// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerLightComponent.h"

#include "VectorUtil.h"
#include "Actors/Player/APlayerCharacter.h"


// Sets default values for this component's properties
UPlayerLightComponent::UPlayerLightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	ProtectionZone = CreateDefaultSubobject<USphereComponent>(FName("ProtectionZone"));
	ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProtectionZone->SetCollisionObjectType(ECC_WorldDynamic);
	ProtectionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProtectionZone->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(FName("LightEmitter"));
	PointLight->SetupAttachment(ProtectionZone);
	PointLight->SetIntensity(0.f);

	LightMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("LanternMesh"));

	// ...
}


void UPlayerLightComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetOwner()->HasAuthority() && bLightOn)
	{
		if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor))
		{
			Player->AddProtectionZone();
		}
	}
}

void UPlayerLightComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GetOwner()->HasAuthority() && bLightOn)
	{
		if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor))
		{
			Player->RemoveProtectionZone();
		}
	}
}

void UPlayerLightComponent::TurnLightOn()
{
	bLightOn = true;
	PointLight->SetIntensity(LightIntensity);
	PointLight->SetSourceRadius(LightRadius);
}

// Called when the game starts
void UPlayerLightComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		ProtectionZone->OnComponentBeginOverlap.AddDynamic(this, &UPlayerLightComponent::OnOverlapBegin);
		ProtectionZone->OnComponentEndOverlap.AddDynamic(this, &UPlayerLightComponent::OnOverlapEnd);
	}
	// ... 
	
}


// Called every frame
void UPlayerLightComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

