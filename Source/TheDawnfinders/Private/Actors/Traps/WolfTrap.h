// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "WolfTrap.generated.h"

UCLASS()
class THEDAWNFINDERS_API AWolfTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	AWolfTrap();
    
	virtual void DoTrapAction() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void BeginPlay() override;

protected:
	// On surcharge cette fonction pour dire au système : "Cet objet est utilisable seulement si y'a un mec dedans"
	virtual bool GetCanBeUsed_Implementation() override;
	
	// Appelé quand le QTE est réussi
	virtual void OnQTESuccess() override;
    
	// Appelé quand le QTE échoue
	virtual void OnQTEFailed() override;
	
	virtual void OnRep_TrappedActor() override;
	
	virtual bool GetQTENeeded_Implementation() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartTrapQTE(AAPlayerCharacter* TargetPlayer);

protected:
	// Temps avant que le joueur piégé puisse se libérer seul
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float TimeBeforeSelfRelease = 5.0f;
    
	// Timer actuel pour le self-release
	float CurrentTrappedTime = 0.0f;
    
	// Le joueur piégé peut-il interagir pour se libérer ?
	UPROPERTY(Replicated,EditAnywhere, BlueprintReadWrite, Category = "Trap")
	bool bCanSelfRelease = false;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Trap")
	bool bCanTrap;
    
	// Libère le joueur piégé
	UFUNCTION(Server, Reliable)
	void Server_ReleaseTrappedActor();
    
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReleaseTrappedActor();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};