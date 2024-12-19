#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DrawingImporter.generated.h"

UCLASS()
class DRAWALIVE_API ADrawingImporter : public AActor
{
	GENERATED_BODY()

public:
	// Konstruktor
	ADrawingImporter();

	// Funkcje
	void ImportDrawing(const FString& FilePath);
	void OpenFileDialogAndImport();
	void ApplyTextureToMesh(class UStaticMeshComponent* MeshComponent, class UTexture2D* Texture);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UTexture2D* ImportedTexture;  // Przechowuje ostatnio zaimportowaną teksturę
};
