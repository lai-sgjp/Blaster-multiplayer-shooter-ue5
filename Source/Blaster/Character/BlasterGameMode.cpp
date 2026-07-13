// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "BlasterCharacter.h"
#include "BlasterPlayerController.h"

ABlasterGameMode::ABlasterGameMode()
{
	// set default pawn class to our character class
	DefaultPawnClass = ABlasterCharacter::StaticClass();
	PlayerControllerClass = ABlasterPlayerController::StaticClass();
}
