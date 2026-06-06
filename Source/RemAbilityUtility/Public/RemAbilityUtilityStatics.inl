// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "Enum/RemHelperEnum.h"
#include "RemNotNull.h"
#include "AbilitySystemComponent.h"
#include "GameplayTag/RemGameplayTagStatics.h"
#include "GameplayTag/RemGameplayTagArray.h"

namespace Rem::GameplayTag
{

template <Enum::ELogicOperator MatchMode = Enum::ELogicOperator::All>
bool HasMatchingGameplayTags(const TNotNull<UAbilitySystemComponent*> AbilitySystem, const FGameplayTagContainer& Tags)
{
    if constexpr (MatchMode == Enum::ELogicOperator::All)
    {
        return AbilitySystem->HasAllMatchingGameplayTags(Tags);
    }
    else if constexpr (MatchMode == Enum::ELogicOperator::Any)
    {
        return AbilitySystem->HasAnyMatchingGameplayTags(Tags);
    }
    else
    {
        static_assert(MatchMode == Enum::ELogicOperator::None);

        return !AbilitySystem->HasAnyMatchingGameplayTags(Tags);
    }
}

template <Enum::ELogicOperator MatchMode = Enum::ELogicOperator::All>
bool HasMatchingGameplayTags(const TNotNull<UAbilitySystemComponent*> AbilitySystem, const FRemGameplayTagArray& Tags)
{
    return HasMatchingGameplayTags<MatchMode>(AbilitySystem, FGameplayTagContainer::CreateFromArray(Tags.Get()));
}

/**
 * FGameplayTagQuery overload for consistent API
 */
template <Enum::ELogicOperator MatchMode = Enum::ELogicOperator::All>
bool HasMatchingGameplayTags(const TNotNull<UAbilitySystemComponent*> AbilitySystem, const FGameplayTagQuery& TagQuery)
{
    return IsTagQueryMatches(TagQuery, AbilitySystem->GetOwnedGameplayTags());
}

/**
 * FGameplayTagQuery overload for consistent API
 */
template <Enum::ELogicOperator MatchMode = Enum::ELogicOperator::All>
bool HasMatchingGameplayTags(const TNotNull<UAbilitySystemComponent*> AbilitySystem, const FGameplayTag& Tag)
{
    return AbilitySystem->HasMatchingGameplayTag(Tag);
}

}
