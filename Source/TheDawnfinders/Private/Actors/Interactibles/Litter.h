#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "Components/BoxComponent.h"
#include "Components/UInventoryComponent.h"
#include "Components/UPlayerLightComponent.h"
#include "Litter.generated.h"


class ASoundManager;
class USphereComponent;
class UPointLightComponent;
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
	void ClampToGround();

public:
    virtual void BeginPlay() override;
	void SmoothClientTransform(float DeltaTime);
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- COMPONENTS ---
    // The invisible physics representation (Root)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Litter")
    UBoxComponent* RootCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Litter")
    UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Litter")
	UPlayerLightComponent* LightComponent;

    // The visual mesh (Attached to Root, allows for rotation offset)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Litter")
    UStaticMeshComponent* MeshComponent;

    // The 4 slots where players attach
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Litter")
    TArray<USceneComponent*> CarryPoints;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	UStaticMeshComponent* LightMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	UPointLightComponent* PointLight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	USphereComponent* ProtectionZone;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	USphereComponent* FogOfWarLightOn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	USphereComponent* FogOfWarLightOff;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class UBoxComponent* FrontTrigger;

	// Zone d'interaction Arrière (Portage)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class UBoxComponent* BackTrigger;

	// Zone d'interaction Centrale (Inventaire)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class UBoxComponent* InventoryTrigger;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	USoundBase* SoloSound;
	
private:
	float LastStrainSoundTime = 0.f; // Pour éviter de spammer le son "trop lourd"
	float BaseMass = 100.f;
	
public:
	// Fonction pour gérer l'affichage du widget selon la zone
	UFUNCTION()
	void OnZoneOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnZoneOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Vérifie si un joueur va toucher quelque chose avec le mouvement prévu
	UFUNCTION()
	bool CheckPlayerCollision(const FVector& DeltaLoc, const FRotator& DeltaRot, FHitResult& OutHit);
private:
	// Pousse le brancard si un joueur est coincé
	void ResolveWallPenetration(float DeltaTime);
protected:
    // --- PHYSICS CONFIGURATION ---
    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float Mass = 100.0f; // Simulate weight (kg)

    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float PushForce = 8000.0f; // Force applied by one player (Newtons * scale)

    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float LinearDamping = 0.8f; // "Friction" for movement (Higher = stops faster)

    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float AngularDamping = 1.5f; // "Friction" for rotation (Higher = stops spinning faster)

    UPROPERTY(EditAnywhere, Category = "Litter Physics")
    float RotationalInertia = 50000.0f;// Resistance to turning (Higher = feels heavier to turn)

	
	

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
	void Server_StartPushing(AAPlayerCharacter* Player, int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_EndPushing(AAPlayerCharacter* Player);

    // Unreliable is faster/better for continuous input updates
    UFUNCTION(Server, Unreliable)
    void Server_UpdateInputs(AAPlayerCharacter* Player, FVector WorldInputDirection);
	
	UPROPERTY(Replicated)
	FTransform ServerTransform;

private:
    // --- INTERNAL LOGIC ---
    void ResolvePhysics(float DeltaTime);
    void AttachPlayerToSlot(AAPlayerCharacter* Player, int32 SlotIndex);
    void DetachPlayer(AAPlayerCharacter* Player);

	UPROPERTY()
	ASoundManager* SoundManagerInstance;
};
