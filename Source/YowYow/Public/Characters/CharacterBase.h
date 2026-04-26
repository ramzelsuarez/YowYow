// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Interfaces/Damageable.h"
#include "CharacterBase.generated.h"

/**
 * this class is the base for all characters in the game, it should implement the following components: (besides obvious movement component etc)
 * - UPaperZDAnimationComponent: self explanatory and needed for all APaperZDCharacter children
 * - UHealthComponent: self explanatory / avoid if we have npcs (not sure if we will have them, but those don't engage in combat  and that would make this base character even simpler)
 * - UAttackComponent: self explanatory / avoid if we have npcs (not sure if we will have them, but those don't attack and that would make this base character even simpler)
 * - USpriteDirectionComponent: component that will cache the player's camera, get its direction, compare it to this character's forward
 *		and generate sprite directionality based on it. The PaperZD ABP should consume from it to display the correct sprite
 */
UCLASS()
class YOWYOW_API ACharacterBase : public APaperZDCharacter, public IDamageable
{
	GENERATED_BODY()
	
};
