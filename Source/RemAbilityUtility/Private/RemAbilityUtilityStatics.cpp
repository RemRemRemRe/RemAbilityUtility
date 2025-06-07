// Copyright RemRemRemRe. 2025. All Rights Reserved.


#include "RemAbilityUtilityStatics.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RemAbilityUtilityStatics)

bool Rem::Ability::HasAnyMatchingQueryTag(const UAbilitySystemComponent& AbilitySystem, const FGameplayTagQuery& TagQuery)
{
	return HasAnyMatchingTags(AbilitySystem, TagQuery.GetGameplayTagArray());
}

bool Rem::Ability::HasAllMatchingQueryTag(const UAbilitySystemComponent& AbilitySystem,
	const FGameplayTagQuery& TagQuery)
{
	return HasAllMatchingTags(AbilitySystem, TagQuery.GetGameplayTagArray());
}

bool Rem::Ability::HasAnyMatchingTags(const UAbilitySystemComponent& AbilitySystem, const TConstArrayView<FGameplayTag> Tags)
{
	if (Tags.IsEmpty())
	{
		return true;
	}

	for (auto& Tag : Tags)
	{
		if (AbilitySystem.GetTagCount(Tag) > 0)
		{
			return true;
		}
	}

	return false;
}

bool Rem::Ability::HasAllMatchingTags(const UAbilitySystemComponent& AbilitySystem, const TConstArrayView<FGameplayTag> Tags)
{
	if (Tags.IsEmpty())
	{
		return true;
	}

	for (auto& Tag : Tags)
	{
		if (AbilitySystem.GetTagCount(Tag) == 0)
		{
			return false;
		}
	}

	return true;
}
