#include "DebugWindow.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/UHealthComponent.h"
#include "Kismet/GameplayStatics.h"       // Pour trouver le PlayerPawn
#include "Public/Actors/Player/APlayerCharacter.h"
#include "Private/GameFramework/SessionManagerSubsystem.h"
#include "Components/DebugComponent.h"
#include "GameFramework/GameModeBase.h"

FDebugWindow::FDebugWindow()
	: FSlateIMWindowBase(TEXT("God Window"), FVector2f(400, 600), TEXT("GodWindow.Toggle"), TEXT("Ouvre ma fenêtre de debug"))
{
	UE_LOG(LogTemp, Warning, TEXT("--- FDebugWindow CONSTRUCTOR CALLED ---"));
}

void FDebugWindow::DrawWindow(float DeltaTime)
{
	UWorld* GameWorld = nullptr;
    
	// --- Sécurité améliorée pour récupérer le monde ---
#if WITH_EDITOR
	if (GEditor && GEditor->PlayWorld) 
	{
		GameWorld = GEditor->PlayWorld;
	}
#endif
	// Fallback si on n'est pas dans l'éditeur
	if (!GameWorld && GWorld)
	{
		GameWorld = GWorld->GetWorld();
	}
	

	// 1. Si le monde n'existe pas ou est en train d'être détruit (IsTearingDown), on arrête TOUT de suite.
	if (!GameWorld || GameWorld->bIsTearingDown)
	{
		SlateIM::Text(TEXT("En attente du jeu..."), FLinearColor::Gray);
		return;
	}
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GameWorld, 0);
	if (!IsValid(PlayerPawn)) 
	{
		return;
	}
	AAPlayerCharacter* PlayerChar = Cast<AAPlayerCharacter>(PlayerPawn);
	if (!PlayerChar || !IsValid(PlayerChar)) 
		return;
	UGameInstance* GameIntance = GameWorld->GetGameInstance();
	if (!GameIntance) return; // CRUCIAL : Le GameInstance peut être null lors de l'arrêt

	USessionManagerSubsystem* SessionManager = GameIntance->GetSubsystem<USessionManagerSubsystem>();
	// SessionManager peut aussi être null si le subsystem a déjà été shut down
    
	UDebugComponent* DebugComp = PlayerChar->FindComponentByClass<UDebugComponent>();
	
	SlateIM::BeginVerticalStack();
	FString HealthText = FString::Printf(
		TEXT("Health : %.1f/%.1f"),
		PlayerChar->HealthComponent->CurrentHealth,
		PlayerChar->HealthComponent->MaxHealth);
	SlateIM::SpinBox(PlayerChar->HealthComponent->CurrentHealth,
		0.0f,
		PlayerChar->HealthComponent->MaxHealth);

	if (SlateIM::Button(TEXT("Revive")))
	{
		PlayerChar->HealthComponent->Server_Revive();
		PlayerChar->HealthComponent->CurrentHealth = PlayerChar->HealthComponent->MaxHealth;
	}
	SlateIM::SpinBox(PlayerChar->PlayerConfig->RunSpeed,0.f,1000.f);
	
	SlateIM::SpinBox(PlayerChar->PlayerConfig->ReloadSpeed,0.f,75.f);
	
	SlateIM::Text(TEXT("=== SESSION MANAGEMENT ==="), FLinearColor::Blue);

	if (SessionManager)
	{
		// BOUTON CRÉER SESSION
		if (SlateIM::Button(TEXT("Create Online Session (Steam)")))
		{
			SessionManager->CreateAdvancesSession(
				4,      // Max Players
				0,      // Private Connections
				false,  // LAN
				true,   // Allow Invites
				false,  // Dedicated
				true    // Use Lobbies
			);
        
			// On voyage vers la map de test en listen server
			UGameplayStatics::OpenLevel(GameWorld, FName("L_GymRoom"), true, "listen");
		}

		// BOUTON INVITATION (Conditionnel : Seulement si une session existe)
		// Note : On utilise la même logique que dans ton Widget
		if (SessionManager->HasActiveSession())
		{
			if (SlateIM::Button(TEXT("Invite Friends (Steam Overlay)")))
			{
				SessionManager->OpenSteamInviteOverlay();
			}
		}
		else 
		{
			SlateIM::Text(TEXT("No Active Session to Invite"), FLinearColor::Red);
		}
	}
	else
	{
		SlateIM::Text(TEXT("Session Manager Unavailable"), FLinearColor::Red);
	}
	if (SlateIM::Button(TEXT("Start")))
	{
		DebugComp->Server_TravelToMap("L_Dungeon_01?listen");
	}
	
	SlateIM::Text(TEXT("=== TELEPORTATION ==="), FLinearColor::Green);
	
	if (SlateIM::Button(TEXT("GymRoom")))
	{
		DebugComp->Server_TravelToMap("L_GymRoom?listen");
	}
	
	if (SlateIM::Button(TEXT("Teleport to Spawn")))
	{
		if (DebugComp)
			DebugComp->Server_TeleportToSpawn();
	}

	SlateIM::Text(TEXT("------------------"), FLinearColor::Gray);

    // --- 2. LOGIQUE DU DROPDOWN --
	
    // Variable statique pour retenir l'état ouvert/fermé entre les frames
    static bool bIsSpawnerOpen = false;
	
    FString DropdownTitle = bIsSpawnerOpen ? TEXT("[-] CACHER SPAWNER") : TEXT("[+] OUVRIR SPAWNER");

    // Si on clique sur le bouton titre, on inverse l'état
    if (SlateIM::Button(*DropdownTitle))
    {
        bIsSpawnerOpen = !bIsSpawnerOpen;
    }
	
	SlateIM::Text(TEXT("Spawnable Items"), FLinearColor::Yellow);
	

    // --- 3. CONTENU DU MENU (Affiché seulement si ouvert) ---
    if (bIsSpawnerOpen)
    {
    	// --- A. Chargement des Assets (Optimisé: fait 1 seule fois) ---
    	static TArray<FAssetData> ItemAssetsList;
    	static bool bAssetsSearched = false;

    	if (!bAssetsSearched)
    	{
    		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    		FARFilter Filter;
            
    		// Méthode Auto pour la classe
    		Filter.ClassPaths.Add(UItemData::StaticClass()->GetClassPathName());
    		// Ton dossier
    		Filter.PackagePaths.Add(FName("/Game/Data/Items")); 
    		Filter.bRecursivePaths = true; 

    		AssetRegistryModule.Get().GetAssets(Filter, ItemAssetsList);
    		bAssetsSearched = true;
    	}

    	// --- B. Boucle d'affichage des boutons ---
    	SlateIM::BeginScrollBox(Orient_Vertical);
    	for (const FAssetData& AssetData : ItemAssetsList)
    	{
    		UItemData* ItemData = Cast<UItemData>(AssetData.GetAsset());
    		if (!ItemData) continue; // Sécurité

    		// On affiche le nom de l'item (ou le nom du fichier si ItemName est vide)
    		FString ItemName = ItemData->ItemName.IsEmpty() ? AssetData.AssetName.ToString() : ItemData->ItemName;
            
    		// Petit style : " > Sword"
    		if (SlateIM::Button(*FString::Printf(TEXT("   > %s"), *ItemName)))
    		{
    			FVector SpawnLoc = PlayerChar->GetActorLocation() + (PlayerChar->GetActorForwardVector() * 150.f) + FVector(0,0,50);
    			FRotator SpawnRot = FRotator::ZeroRotator;

    			// 3. Appel RPC via le composant
    			if (DebugComp)
    			{
    				DebugComp->Server_SpawnDebugItem(ItemData, SpawnLoc, SpawnRot);
    				UE_LOG(LogTemp, Log, TEXT("Ordre de spawn envoyé via DebugComponent"));
    			}
    			else
    			{
    				UE_LOG(LogTemp, Warning, TEXT("DebugComponent introuvable !"));
    			}
    		}
    	}
    	SlateIM::EndScrollBox();
    }
	SlateIM::EndVerticalStack();
}