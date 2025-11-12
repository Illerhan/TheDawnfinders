// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlayerLightComponent.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/UHealthComponent.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UPlayerLightComponent::UPlayerLightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	LightRoot = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	
	PointLight = CreateDefaultSubobject<UPointLightComponent>(FName("Light"));
	PointLight->SetupAttachment(LightRoot);
	
	ProtectionZone = CreateDefaultSubobject<USphereComponent>(FName("ProtectionZone"));
	ProtectionZone->SetGenerateOverlapEvents(true);
	ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProtectionZone->SetCollisionObjectType(ECC_WorldDynamic);
	ProtectionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProtectionZone->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	ProtectionZone->SetHiddenInGame(!bLightOn);
	ProtectionZone->SetupAttachment(PointLight);
	
	
	
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
			Player->HealthComponent->AddProtectionZone();
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] OVERLAP BEGIN - %s entered %s protection zone. Amount: %d"),
				*Player->GetName(),
				*GetOwner()->GetName(),
				Player->HealthComponent->ProtectionZoneAmount);
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
			Player->HealthComponent->RemoveProtectionZone();
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] OVERLAP END - %s left %s protection zone. Amount: %d"),
				*Player->GetName(),
				*GetOwner()->GetName(),
				Player->HealthComponent->ProtectionZoneAmount);
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

			// Self-protection add
			if (AAPlayerCharacter* OwnerPlayer = Cast<AAPlayerCharacter>(GetOwner()))
			{
				OwnerPlayer->HealthComponent->AddProtectionZone();
				UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s turned light ON - Self protection. Amount: %d"),
					*OwnerPlayer->GetName(),
					OwnerPlayer->HealthComponent->ProtectionZoneAmount);
			}
			
			// Other protection add (EXCLUDE OWNER)
			if (ProtectionZone && GetOwner()->HasAuthority())
			{
				TArray<AActor*> OverlappingActors;
				ProtectionZone->GetOverlappingActors(OverlappingActors, AAPlayerCharacter::StaticClass());
                
				for (AActor* Actor : OverlappingActors)
				{
					// SKIP THE OWNER - they already got self-protection above
					if (Actor == GetOwner()) continue;
					
					if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Actor))
					{
						Player->HealthComponent->AddProtectionZone();
						UE_LOG(LogTemp, Warning, TEXT("[SERVER] Light turned ON - Adding protection for %s. Amount: %d"),
							*Player->GetName(),
							Player->HealthComponent->ProtectionZoneAmount);
					}
				}
			}
		}
		else
		{
			PointLight->SetIntensity(0.f);
			PointLight->SetVisibility(false);
			
			if (ProtectionZone && GetOwner()->HasAuthority())
			{
				// Self-protection removed
				if (AAPlayerCharacter* OwnerPlayer = Cast<AAPlayerCharacter>(GetOwner()))
				{
					OwnerPlayer->HealthComponent->RemoveProtectionZone();
					UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s turned light OFF - Removing self protection. Amount: %d"),
						*OwnerPlayer->GetName(),
						OwnerPlayer->HealthComponent->ProtectionZoneAmount);
				}

				// Other protection removed (EXCLUDE OWNER)
				TArray<AActor*> OverlappingActors;
				ProtectionZone->GetOverlappingActors(OverlappingActors, AAPlayerCharacter::StaticClass());
                
				for (AActor* Actor : OverlappingActors)
				{
					// SKIP THE OWNER - they already got self-protection removed above
					if (Actor == GetOwner()) continue;
					
					if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Actor))
					{
						Player->HealthComponent->RemoveProtectionZone();
						UE_LOG(LogTemp, Warning, TEXT("[SERVER] Light turned OFF - Removing protection for %s. Amount: %d"),
							*Player->GetName(),
							Player->HealthComponent->ProtectionZoneAmount);
					}
				}
			}
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
			UE_LOG(LogTemp, Error, TEXT("  - Player: %s, Protection Amount: %d"),
				*Player->GetName(),
				Player->HealthComponent->ProtectionZoneAmount);
		}
	}
}