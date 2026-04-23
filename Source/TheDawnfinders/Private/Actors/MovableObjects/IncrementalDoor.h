 #pragma once
  
  
  #include "CoreMinimal.h"
  
  #include "Actors/MovableObjects/Doors.h"
  
  #include "IncrementalDoor.generated.h"
  
  
  UCLASS()
  
  class THEDAWNFINDERS_API AIncrementalDoor : public ADoors
  {
      GENERATED_BODY()

  public:
      AIncrementalDoor();

  protected:
      virtual void BeginPlay() override;
      virtual void Tick(float DeltaTime) override;

      // Intercepte l'appel du Lever
      virtual void StartOpening() override;
      virtual void StopOpening() override;
      virtual void StopMainAction_Implementation() override;

  private:
      int32 ActivationCount = 0;
      float TargetCurvePos  = 0.f;
      bool  bMovingToStep   = false;
  };