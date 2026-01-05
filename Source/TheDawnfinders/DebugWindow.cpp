// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugWindow.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/UHealthComponent.h"
#include "Kismet/GameplayStatics.h"       // Pour trouver le PlayerPawn
#include "Editor/EditorEngine.h"          // Pour accéder à GEditor
#include "Public/Actors/Player/APlayerCharacter.h"
#include "Private/GameFramework/SessionManagerSubsystem.h"
#include "Components/DebugComponent.h"

FDebugWindow::FDebugWindow()
	: FSlateIMWindowBase(TEXT("Ma Fenêtre Debug"), FVector2f(400, 600), TEXT("MyDebug.Toggle"), TEXT("Ouvre ma fenêtre de debug"))
{
	UE_LOG(LogTemp, Warning, TEXT("--- FDebugWindow CONSTRUCTOR CALLED ---"));
}

void FDebugWindow::DrawWindow(float DeltaTime)
{
	UWorld* GameWorld = GEditor ? GEditor->PlayWorld : nullptr;
	if (!GameWorld)
	{
		SlateIM::Text(TEXT("Le jeu n'est pas lancé. En attente..."), FLinearColor::Gray);
		return; // On arrête là, pas de joueur à chercher
	}
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GameWorld, 0);

	AAPlayerCharacter* PlayerChar = Cast<AAPlayerCharacter>(PlayerPawn);

	if (!PlayerChar)
		return;
	UGameInstance* GameIntance = GameWorld->GetGameInstance();
	USessionManagerSubsystem* SessionManager = GameIntance->GetSubsystem<USessionManagerSubsystem>();;
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
	
	if (SlateIM::Button(TEXT("CreateSession")))
	{
		SessionManager->CreateAdvancesSession(
		4,
		4,
		false,  
		true, 
		false,
		true    
	);
		UGameplayStatics::OpenLevel(GameWorld, FName("Lvl_TopDown"),false,"listen?");
	}
	if (SlateIM::Button(TEXT("Start")))
	{
		GameWorld->ServerTravel("L_Prototype?listen",true);
	}

	SlateIM::Text(TEXT("------------------"), FLinearColor::Gray);

    // --- 2. LOGIQUE DU DROPDOWN ---
    
    // Variable statique pour retenir l'état ouvert/fermé entre les frames
    static bool bIsSpawnerOpen = false;

    // Le texte du bouton change selon l'état (+ ou -)
    FString DropdownTitle = bIsSpawnerOpen ? TEXT("[-] CACHER SPAWNER") : TEXT("[+] OUVRIR SPAWNER");

    // Si on clique sur le bouton titre, on inverse l'état
    if (SlateIM::Button(*DropdownTitle))
    {
        bIsSpawnerOpen = !bIsSpawnerOpen;
    }

    // --- 3. CONTENU DU MENU (Affiché seulement si ouvert) ---
    if (bIsSpawnerOpen)
    {
	    // On décale un peu visuellement (si SlateIM a un Indent, sinon pas grave)
    	// SlateIM::Indent(10.f); 

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

    			// 2. On cherche le composant sur le Controller (ou le Pawn selon où tu l'as mis)
    			UDebugComponent* DebugComp = PlayerChar ? PlayerChar->FindComponentByClass<UDebugComponent>() : nullptr;

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
    }
	SlateIM::EndVerticalStack();
}