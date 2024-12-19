#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ShapeType.h"
#include "ShapePickerWidget.generated.h"

// Delegate to notify about shape selection with image file path
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShapeSelected, EShapeType, ShapeType, FString, ImageFilePath);

UCLASS()
class DRAWALIVE_API UShapePickerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShapeSelected OnShapeSelected;

	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* CubeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SphereButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ConeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CylinderButton;

	UFUNCTION()
	void SelectShape(EShapeType ShapeType);

	// Functions for each button
	UFUNCTION()
	void OnCubeButtonClicked();

	UFUNCTION()
	void OnSphereButtonClicked();

	UFUNCTION()
	void OnConeButtonClicked();

	UFUNCTION()
	void OnCylinderButtonClicked();
};
