// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/PlayerLightComponent.h"
#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/CustomPlayerState.h"
#include "Components/UHealthComponent.h"
#include "Net/UnrealNetwork.h"

UPlayerLightComponent::UPlayerLightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	LightRoot = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	
	PointLight = CreateDefaultSubobject<UPointLightComponent>(FName("Light"));
	PointLight->SetupAttachment(LightRoot);

	ProtectionZone = CreateDefaultSubobject<USphereComponent>(FName("ProtectionZone"));
	ProtectionZone->SetGenerateOverlapEvents(true);
	ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProtectionZone->SetCollisionObjectType(ECC_WorldDynamic);
	ProtectionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProtectionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ProtectionZone->SetHiddenInGame(true);
	ProtectionZone->SetupAttachment(PointLight);

	LightMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("LanternMesh"));
	LightMesh->SetupAttachment(LightRoot);

	bLightOn = false;
	FuelRemaining = 100.f;
	LightIntensity = 5000.f;
	LightRadius = 300.f;
	FuelConsumption = 5.f;

	SetIsReplicatedByDefault(true);
}

void UPlayerLightComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerLightComponent, bLightOn);
	DOREPLIFETIME(UPlayerLightComponent, FuelRemaining);
}

void UPlayerLightComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (LightRoot)
		{
			LightRoot->AttachToComponent(
				Owner->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform
			);
		}
	}
	
	LightRoot->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	if (GetOwnerRole() == ROLE_Authority)
	{
		ProtectionZone->OnComponentBeginOverlap.AddDynamic(this, &UPlayerLightComponent::OnOverlapBegin);
		ProtectionZone->OnComponentEndOverlap.AddDynamic(this, &UPlayerLightComponent::OnOverlapEnd);
	}

	ApplyLightState();


	// Actualises the local UI
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseLocalLantern(FuelRemaining, MaxFuel);
}


void UPlayerLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bLightOn)
	{
		ConsumeFuel(DeltaTime);
	}
}

void UPlayerLightComponent::ConsumeFuel(float DeltaTime)
{
	FuelRemaining -= FuelConsumption * DeltaTime;
	FuelRemaining = FMath::Max(FuelRemaining, 0.f);

	if (FuelRemaining <= 0.f && bLightOn)
	{
		TurnLightOff();
	}

	// Actualises the local UI
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseLocalLantern(FuelRemaining, MaxFuel);
}

void UPlayerLightComponent::TurnLightOn()
{
	if (bLightOn) return;

	if (GetOwner()->HasAuthority())
	{
		bLightOn = true;
		ApplyLightState();
	}
	else
	{
		Server_TurnLightOn();
	}
}

void UPlayerLightComponent::TurnLightOff()
{
	if (!bLightOn) return;

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
	TurnLightOn();
}

void UPlayerLightComponent::Server_TurnLightOff_Implementation()
{
	TurnLightOff();
}

void UPlayerLightComponent::OnRep_LightOn()
{
	ApplyLightState();
}

void UPlayerLightComponent::ApplyLightState()
{
	if (!PointLight) return;

	PointLight->SetVisibility(bLightOn);
	PointLight->SetIntensity(bLightOn ? LightIntensity : 0.f);
	PointLight->SetSourceRadius(bLightOn ? LightRadius : 0.f);
	ProtectionZone->SetHiddenInGame(!bLightOn);


	if (!GetOwner()->HasAuthority()) return;

	if (AAPlayerCharacter* OwnerPlayer = Cast<AAPlayerCharacter>(GetOwner()))
	{
		if (bLightOn) OwnerPlayer->HealthComponent->AddProtectionZone();
		else OwnerPlayer->HealthComponent->RemoveProtectionZone();
	}

	if (!ProtectionZone) return;

	TArray<AActor*> OverlappingActors;
	ProtectionZone->GetOverlappingActors(OverlappingActors, AAPlayerCharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == GetOwner()) continue;

		if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Actor))
		{
			if (bLightOn) Player->HealthComponent->AddProtectionZone();
			else Player->HealthComponent->RemoveProtectionZone();
		}
	}
}

void UPlayerLightComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetOwner()->HasAuthority() || !bLightOn) return;

	if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor))
	{
		if (Player != GetOwner())
		{
			Player->HealthComponent->AddProtectionZone();
		}
	}
}

void UPlayerLightComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetOwner()->HasAuthority() || !bLightOn) return;

	if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor))
	{
		if (Player != GetOwner())
		{
			Player->HealthComponent->RemoveProtectionZone();
		}
	}
}

void UPlayerLightComponent::DebugProtectionZone()
{
	if (!ProtectionZone) return;

	TArray<AActor*> OverlappingActors;
	ProtectionZone->GetOverlappingActors(OverlappingActors);

	UE_LOG(LogTemp, Error, TEXT("=== DEBUG %s Protection Zone ==="), *GetOwner()->GetName());
	UE_LOG(LogTemp, Error, TEXT("Light On: %s"), bLightOn ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Error, TEXT("Collision Enabled: %d"), (int32)ProtectionZone->GetCollisionEnabled());
	UE_LOG(LogTemp, Error, TEXT("Overlapping Actors: %d"), OverlappingActors.Num());

	for (AActor* Actor : OverlappingActors)
	{
		if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Actor))
		{

		}
	}
}
