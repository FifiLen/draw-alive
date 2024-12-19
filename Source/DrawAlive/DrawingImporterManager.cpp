#include "DrawingImporterManager.h"
#include "DrawingImporter.h"

// Sets default values
ADrawingImporterManager::ADrawingImporterManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADrawingImporterManager::BeginPlay()
{
	Super::BeginPlay();

	// Create DrawingImporter when the game starts
	CreateDrawingImporter();
}

void ADrawingImporterManager::CreateDrawingImporter()
{
	FVector Location(0.0f, 0.0f, 100.0f);
	FActorSpawnParameters SpawnParams;

	// Spawn a new DrawingImporter actor in the level
	ADrawingImporter* NewDrawingImporter = GetWorld()->SpawnActor<ADrawingImporter>(ADrawingImporter::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);

	if (NewDrawingImporter)
	{
		UE_LOG(LogTemp, Warning, TEXT("DrawingImporter actor created successfully!"));
		// NewDrawingImporter->ImportDrawing(TEXT("/Path/To/Your/Image.png")); // Ścieżka do pliku do załadowania
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create DrawingImporter actor!"));
	}
}
