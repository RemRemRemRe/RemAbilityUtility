// Copyright RemRemRemRe. 2024. All Rights Reserved.


#include "RemTagEventManager.h"

#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RemTagEventManager)

FRemTagEventManager::FRemTagEventManager(UAbilitySystemComponent* InAbilitySystem)
	: AbilitySystem(InAbilitySystem)
{
}

void FRemTagEventManager::RegisterGameplayTagEventUnique(const FGameplayTag Tag, const EGameplayTagEventType::Type EventType,
	const FDelegate& Delegate)
{
	if (!TagToEventHandle.Contains(Tag))
	{
		const auto Handle = AbilitySystem->RegisterGameplayTagEvent(Tag, EventType).Add(Delegate);

		TagToEventHandle.Add(Tag, Handle);
	}
}

void FRemTagEventManager::RegisterGameplayTagEventUnique(const FGameplayTag Tag, const FDelegate& Delegate)
{
	return RegisterGameplayTagEventUnique(Tag, EGameplayTagEventType::NewOrRemoved, Delegate);
}

void FRemTagEventManager::RegisterGameplayTagEvent(const FGameplayTag Tag, const EGameplayTagEventType::Type EventType,
	const FDelegate& Delegate)
{
	const auto Handle = AbilitySystem->RegisterGameplayTagEvent(Tag, EventType).Add(Delegate);

	TagToEventHandle.FindOrAdd(Tag, Handle);
}

void FRemTagEventManager::RegisterGameplayTagEvent(const FGameplayTag Tag,
	const FDelegate& Delegate)
{
	return RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved, Delegate);
}

void FRemTagEventManager::Reset()
{
	SetAbilitySystem(nullptr);
}

void FRemTagEventManager::UnRegisterGameplayTagEvents()
{
	if (AbilitySystem.IsValid())
	{
		for (const auto& Data : TagToEventHandle)
		{
			AbilitySystem->RegisterGameplayTagEvent(Data.Key).Remove(Data.Value);
		}
	}

	TagToEventHandle.Reset();
}

bool FRemTagEventManager::IsValid() const
{
	return AbilitySystem.IsValid();
}

FRemTagEventManager::~FRemTagEventManager()
{
	UnRegisterGameplayTagEvents();
}

bool FRemTagEventManager::SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
	if (InAbilitySystem != AbilitySystem)
	{
		UnRegisterGameplayTagEvents();

		AbilitySystem = InAbilitySystem;

		return true;
	}

	return false;
}
