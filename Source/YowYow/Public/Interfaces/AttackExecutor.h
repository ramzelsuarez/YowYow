#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/AttackTypes.h"
#include "AttackExecutor.generated.h"

class UAttackComponent;

UINTERFACE(BlueprintType)
class YOWYOW_API UAttackExecutor : public UInterface
{
	GENERATED_BODY()
};

class YOWYOW_API IAttackExecutor
{
	GENERATED_BODY()

public:
	// The future yo-yo component should return true for the attack shapes it owns.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Combat")
	bool CanExecuteAttack(const FAttackData& AttackData) const;

	// Call AttackComponent->FinishExternalAttack(this) when the movement ends.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Combat")
	void ExecuteAttack(UAttackComponent* AttackComponent, const FAttackData& AttackData);
};
