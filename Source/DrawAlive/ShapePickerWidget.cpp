#include "ShapePickerWidget.h"
#include "YourGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"

void UShapePickerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Assign events to buttons
	if (CubeButton)
	{
		CubeButton->OnClicked.AddDynamic(this, &UShapePickerWidget::OnCubeButtonClicked);
	}
	if (SphereButton)
	{
		SphereButton->OnClicked.AddDynamic(this, &UShapePickerWidget::OnSphereButtonClicked);
	}
	if (ConeButton)
	{
		ConeButton->OnClicked.AddDynamic(this, &UShapePickerWidget::OnConeButtonClicked);
	}
	if (CylinderButton)
	{
		CylinderButton->OnClicked.AddDynamic(this, &UShapePickerWidget::OnCylinderButtonClicked);
	}
}

void UShapePickerWidget::SelectShape(EShapeType ShapeType)
{
	// Get the Game Mode to call `OpenFileDialog`
	AYourGameMode* GameMode = Cast<AYourGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		FString SelectedFile = GameMode->OpenFileDialog();
		if (!SelectedFile.IsEmpty())
		{
			// Broadcast the shape type and image file path
			OnShapeSelected.Broadcast(ShapeType, SelectedFile);
		}
	}
}

void UShapePickerWidget::OnCubeButtonClicked()
{
	SelectShape(EShapeType::Cube);
}

void UShapePickerWidget::OnSphereButtonClicked()
{
	SelectShape(EShapeType::Sphere);
}

void UShapePickerWidget::OnConeButtonClicked()
{
	SelectShape(EShapeType::Cone);
}

void UShapePickerWidget::OnCylinderButtonClicked()
{
	SelectShape(EShapeType::Cylinder);
}
