// Copyright RemRemRemRe. 2025. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "RemNotNull.h"
#include "RemAbilityUtilityStatics.generated.h"

struct FGameplayTag;
struct FGameplayTagQuery;
class UAbilitySystemComponent;

UCLASS()
class REMABILITYUTILITY_API URemAbilityUtilityStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

};

namespace Rem::Ability
{
[[nodiscard]] REMABILITYUTILITY_API bool HasAnyMatchingQueryTag(TNotNull<const UAbilitySystemComponent*> AbilitySystem,
    const FGameplayTagQuery& TagQuery);
[[nodiscard]] REMABILITYUTILITY_API bool HasAllMatchingQueryTag(TNotNull<const UAbilitySystemComponent*> AbilitySystem,
    const FGameplayTagQuery& TagQuery);

[[nodiscard]] REMABILITYUTILITY_API bool HasAnyMatchingTags(TNotNull<const UAbilitySystemComponent*> AbilitySystem,
    TConstArrayView<FGameplayTag> Tags);
[[nodiscard]] REMABILITYUTILITY_API bool HasAllMatchingTags(TNotNull<const UAbilitySystemComponent*> AbilitySystem,
    TConstArrayView<FGameplayTag> Tags);
}
