#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "Components/BoxComponent.h"
#include "Litter.generated.h"

class AAPlayerCharacter;

USTRUCT()
struct FPusherData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector InputVector = FVector::ZeroVector;

	UPROPERTY()
	float LastUpdateTime = 0.f;
};

UCLASS()
class THEDAWNFINDERS_API ALitter : public AInteractibleObjects
{
	GENERATED_BODY()

public:
    ALitter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- COMPONENTS ---
    // The invisible physics representation (Root)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Litter")
    UBoxComponent* RootCollision;

    // The visual mesh (Attached to Root, allows for rotation offset)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Litter")
    UStaticMeshComponent* MeshComponent;

    // The 4 slots where players attach
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Litter")
    TArray<USceneComponent*> CarryPoints;

	// Vérifie si un joueur va toucher quelque chose avec le mouvement prévu
	UFUNCTION()
	bool CheckPlayerCollision(const FVector& DeltaLoc, const FRotator& DeltaRot, FHitResult& OutHit);
private:
	// Pousse le brancard si un joueur est coincé
	void ResolveWallPenetration(float DeltaTime);
protected:
    // --- PHYSICS CONFIGURATION ---
    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float Mass = 50.0f; // Simulate weight (kg)

    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float PushForce = 150000.0f; // Force applied by one player (Newtons * scale)

    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float LinearDamping = 0.8f; // "Friction" for movement (Higher = stops faster)

    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float AngularDamping = 1.5f; // "Friction" for rotation (Higher = stops spinning faster)

    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float RotationalInertia = 50000.0f; // Resistance to turning (Higher = feels heavier to turn)

    // --- STATE VARIABLES ---
    // Replicated so clients can smooth out movement (Client-side prediction optional)
    UPROPERTY(Replicated)
    FVector CurrentLinearVelocity;

    UPROPERTY(Replicated)
    float CurrentAngularVelocityYaw;

    // Server-side tracking of players
    TMap<AAPlayerCharacter*, FPusherData> ActivePushers;
    TArray<TWeakObjectPtr<AAPlayerCharacter>> CarrySlots; // Array of 4 slots

    float InputTimeout = 0.5f; // Time before a player is considered "stopped" if no input received

	// Force du freinage quand personne ne pousse (plus c'est haut, plus l'arrêt est sec)
	UPROPERTY(EditAnywhere, Category = "Litter Physics")
	float BrakingDeceleration = 2000.0f; 

	// Seuil de vitesse pour arrêt complet (pour éviter les micro-glissements)
	float StopThreshold = 10.0f;

public:
    // --- INTERACTION API ---
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void Interact(AActor* Interactor);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void StopInteract(AActor* Interactor);

    // --- NETWORKED INPUT ---
    UFUNCTION(Server, Reliable)
    void Server_StartPushing(AAPlayerCharacter* Player);

    UFUNCTION(Server, Reliable)
    void Server_EndPushing(AAPlayerCharacter* Player);

    // Unreliable is faster/better for continuous input updates
    UFUNCTION(Server, Unreliable)
    void Server_UpdateInputs(AAPlayerCharacter* Player, FVector WorldInputDirection);

private:
    // --- INTERNAL LOGIC ---
    void ResolvePhysics(float DeltaTime);
    void AttachPlayerToSlot(AAPlayerCharacter* Player, int32 SlotIndex);
    void DetachPlayer(AAPlayerCharacter* Player);
};
