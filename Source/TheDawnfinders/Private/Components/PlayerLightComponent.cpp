// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlayerLightComponent.h"

#include <gsl/pointers>

#include "VectorUtil.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UPlayerLightComponent::UPlayerLightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	LightRoot = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	
	ProtectionZone = CreateDefaultSubobject<USphereComponent>(FName("ProtectionZone"));
	ProtectionZone->SetupAttachment(LightRoot);
	
	ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProtectionZone->SetCollisionObjectType(ECC_WorldDynamic);
	ProtectionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProtectionZone->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(FName("Light"));
	PointLight->SetupAttachment(LightRoot);
	
	LightMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("LanternMesh"));
	LightMesh->SetupAttachment(LightRoot);
	
	bLightOn = false;
	SetIsReplicatedByDefault(true);

}

auto UPlayerLightComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const -> void
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(UPlayerLightComponent, bLightOn);
}

void UPlayerLightComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetOwner()->HasAuthority() && bLightOn)
	{
		if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor))
		{
			Player->HealthComponent()->AddProtectionZone(this);
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
			Player->HealthComponent()->RemoveProtectionZone();
		}
	}
}

void UPlayerLightComponent::ApplyLightState()
{
	
	if (PointLight)
	{
		if (bLightOn)
		{
			PointLight->SetIntensity(LightIntensity);
			PointLight->SetSourceRadius(LightRadius);
			PointLight->SetVisibility(true);
		}
		else
		{
			PointLight->SetIntensity(0.f);
			PointLight->SetVisibility(false);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s Light is now %s"), 
		   *GetOwner()->GetName(), 
		   bLightOn ? TEXT("ON") : TEXT("OFF"));
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
	ApplyLightState();
}


// Called every frame
void UPlayerLightComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerLightComponent::TurnLightOn()
{
	// Le client appelle cette fonction, qui demande au serveur de faire le changement
	if (GetOwner()->HasAuthority())
	{
		// Si on est déjà sur le serveur, changer directement
		bLightOn = true;
		ApplyLightState();
	}
	else
	{
		// Sinon, demander au serveur
		Server_TurnLightOn();
	}
}

void UPlayerLightComponent::TurnLightOff()
{
	if (GetOwner()->HasAuthority())
	{
		bLightOn = false;
		ApplyLightState();
	}
	else
	{
		Server_TurnLightOff();
	}
}
void UPlayerLightComponent::Server_TurnLightOn_Implementation()
{
	bLightOn = true;
	ApplyLightState();
}

void UPlayerLightComponent::Server_TurnLightOff_Implementation()
{
	bLightOn = false;
	ApplyLightState();
}

void UPlayerLightComponent::OnRep_LightOn()
{
	// Appelé sur les clients quand bLightOn change
	ApplyLightState();
}

