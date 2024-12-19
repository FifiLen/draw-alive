#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShapePickerWidget.h"
#include "ShapeType.h"
#include "YourGameMode.generated.h"

UCLASS()
class DRAWALIVE_API AYourGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AYourGameMode();

	// Publiczne funkcje
	FString OpenFileDialog();
	UTexture2D* LoadTextureFromFile(const FString& FilePath);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shapes")
	TSubclassOf<class UShapePickerWidget> ShapePickerWidgetClass;

	UFUNCTION()
	void SpawnShape(EShapeType ShapeType, FString ImageFilePath);

private:
	UPROPERTY()
	UShapePickerWidget* ShapePickerWidgetInstance;

	// Meshe kształtów
	UPROPERTY()
	UStaticMesh* CubeStaticMesh;

	UPROPERTY()
	UStaticMesh* SphereStaticMesh;

	UPROPERTY()
	UStaticMesh* ConeStaticMesh;

	UPROPERTY()
	UStaticMesh* CylinderStaticMesh;

	// Mesh płaszczyzny wody
	UPROPERTY()
	UStaticMesh* WaterPlaneMesh;

	// Materiały
	UPROPERTY()
	UMaterial* DefaultMaterial;

	UPROPERTY()
	UMaterial* WaterMaterial;

	// System cząstek
	UPROPERTY()
	UParticleSystem* BubblesFX;

	// Granice obszaru ruchu
	FVector AreaMin;
	FVector AreaMax;

	void EnableFloatingMovement(AStaticMeshActor* ShapeActor);
	void CreateWallOrFloor(FVector Location, FVector Scale);

	UPROPERTY()
	AStaticMeshActor* LastSpawnedShape;
};
