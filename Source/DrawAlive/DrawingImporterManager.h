#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DrawingImporterManager.generated.h"

UCLASS()
class DRAWALIVE_API ADrawingImporterManager : public AActor
{
	GENERATED_BODY()

public:    
	// Sets default values for this actor's properties
	ADrawingImporterManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:    
	// Function to manage the creation of a drawing importer
	void CreateDrawingImporter();
};
