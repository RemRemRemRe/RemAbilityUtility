// Copyright RemRemRemRe. 2025. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "RemAbilityUtilityStatics.generated.h"

struct FGameplayTag;
struct FGameplayTagQuery;
class UAbilitySystemComponent;

UCLASS()
class REMABILITYUTILITY_API URemAbilityUtilityStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
};

namespace Rem::Ability
{
	REMABILITYUTILITY_API bool HasAnyMatchingQueryTag(const UAbilitySystemComponent& AbilitySystem, const FGameplayTagQuery& TagQuery);
	REMABILITYUTILITY_API bool HasAllMatchingQueryTag(const UAbilitySystemComponent& AbilitySystem, const FGameplayTagQuery& TagQuery);

	REMABILITYUTILITY_API bool HasAnyMatchingTags(const UAbilitySystemComponent& AbilitySystem, TConstArrayView<FGameplayTag> Tags);
	REMABILITYUTILITY_API bool HasAllMatchingTags(const UAbilitySystemComponent& AbilitySystem, TConstArrayView<FGameplayTag> Tags);
}
