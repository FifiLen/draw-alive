#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum class EShapeType : uint8
{
	Cube,
	Sphere,
	Cone,
	Cylinder
};