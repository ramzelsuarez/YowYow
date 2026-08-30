// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MainMenu.h"
#include "Characters/EriCharacter.h"

void AMainMenu::BeginPlay()
{
	Super::BeginPlay();
	AEriCharacter::PreloadCharacterAssets();
}

