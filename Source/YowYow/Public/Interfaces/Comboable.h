#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Comboable.generated.h"

UINTERFACE(MinimalAPI)
class UComboable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Actors that grant stylish-rank combo points when Eri hits them.
 * Enemies implement this. Props (vending machines, etc.) do not.
 */
class YOWYOW_API IComboable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CanGrantCombo() const;
};
