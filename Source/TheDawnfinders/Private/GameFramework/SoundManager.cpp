#include "SoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

// Constructeur : Configuration Réseau
ASoundManager::ASoundManager()
{
	PrimaryActorTick.bCanEverTick = false; // Désactivé pour la performance

	// --- CONFIGURATION RÉSEAU INDISPENSABLE ---
	bReplicates = true;
	bAlwaysRelevant = true; // Permet aux clients de "voir" le manager même de loin
	AActor::SetReplicateMovement(false); 
}

void ASoundManager::BeginPlay()
{
	Super::BeginPlay();
}

// Reçu par le serveur, renvoyé à tout le monde via Multicast
void ASoundManager::ServerPlaySound_Implementation(USoundBase* Sound, FVector Location, float Loudness, float Range, bool bNeedNoise)
{
	MultiPlaySound(Sound, Location, Loudness, Range, bNeedNoise);
}

// Exécuté sur TOUTES les instances (Serveur + tous les Clients)
void ASoundManager::MultiPlaySound_Implementation(USoundBase* Sound, FVector Location, float Loudness, float Range, bool bNeedNoise)
{
	if (Sound)
	{
		// Joue le son localement sur chaque machine
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, Loudness);

		// Seul le serveur gère la perception IA (pour éviter les doublons de calcul)
		if (bNeedNoise && HasAuthority())
		{
			UAISense_Hearing::ReportNoiseEvent(this, Location, Loudness, this, Range);
		}
	}
}