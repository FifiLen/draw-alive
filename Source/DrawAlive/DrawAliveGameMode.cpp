// Copyright Epic Games, Inc. All Rights Reserved.

#include "DrawAliveGameMode.h"
#include "DrawAliveCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADrawAliveGameMode::ADrawAliveGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
