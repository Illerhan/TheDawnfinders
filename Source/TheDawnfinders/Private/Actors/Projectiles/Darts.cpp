// Fill out your copyright notice in the Description page of Project Settings.


#include "Darts.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/UItemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ADarts::ADarts()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SphereCollider = CreateDefaultSubobject<USphereComponent>("SphereCollider");
	SphereCollider->SetupAttachment(RootComponent);
	SphereCollider->SetSphereRadius(50);
	
	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComponent");
	
}

// Called when the game starts or when spawned
void ADarts::BeginPlay()
{
	Super::BeginPlay();
	SphereCollider->OnComponentBeginOverlap.AddDynamic(this,&ADarts::OnOverlapBegin);
	SphereCollider->OnComponentEndOverlap.AddDynamic(this,&ADarts::OnOverlapEnd);
	SetLifeSpan(5.f);
}


void ADarts::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	

}

void ADarts::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                            int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	if (OtherActor->Implements<UDamageable>())
	{
		AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
		if (Player)
		{
			Player->ItemComponent->Server_StartTimedEffect(EConsumableEffectType::Poison,10);
			Destroy();
			return;
		}
		ABaseEnemy* Enemy = Cast<ABaseEnemy>(OtherActor);
		if (Enemy)
		{
			Enemy->Execute_ReceiveDamage(Enemy,50,this);
			Destroy();
		}
	}
}

void ADarts::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
}

// Called every frame
void ADarts::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

