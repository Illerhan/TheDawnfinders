#include "GameplaySoundHelper.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "SoundManager.h"

void UGameplaySoundHelper::PlaySoundNetworked(UObject* WorldContextObject, USoundBase* Sound, FVector Location, float Loudness, float Range, bool bNeedNoise)
{
	if (!WorldContextObject || !Sound) return;

	// Récupération propre du World
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return;

	ASoundManager* Manager = nullptr;

	// 1. On cherche si un Manager existe déjà
	for (TActorIterator<ASoundManager> It(World); It; ++It)
	{
		Manager = *It;
		break;
	}

	// 2. Si aucun manager n'existe, SEUL le serveur a le droit d'en spawn un
	if (!Manager && World->GetNetMode() < NM_Client)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Manager = World->SpawnActor<ASoundManager>(ASoundManager::StaticClass(), Location, FRotator::ZeroRotator, Params);
	}

	if (Manager)
	{
		Manager->ServerPlaySound(Sound, Location, Loudness, Range, bNeedNoise);
	}
}