// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "RemAbilityGameplayTagEventSource.h"

#include "AbilitySystemComponent.h"

FRemAbilityGameplayTagEventSource::FRemAbilityGameplayTagEventSource(UAbilitySystemComponent* InAbilitySystem)
    : AbilitySystem(InAbilitySystem)
{
}

FRemAbilityGameplayTagEventSource::~FRemAbilityGameplayTagEventSource()
{
    UnregisterAllTagEvents();
}

const FGameplayTagContainer& FRemAbilityGameplayTagEventSource::GetOwnedTags() const
{
    static const FGameplayTagContainer EmptyTags{};

    return AbilitySystem ? AbilitySystem->GetOwnedGameplayTags() : EmptyTags;
}

void FRemAbilityGameplayTagEventSource::AddOwnedTags(const FGameplayTagContainer& Tags)
{
    if (AbilitySystem)
    {
        AbilitySystem->AddLooseGameplayTags(Tags);
    }
}

void FRemAbilityGameplayTagEventSource::RemoveOwnedTags(const FGameplayTagContainer& Tags)
{
    if (AbilitySystem)
    {
        AbilitySystem->RemoveLooseGameplayTags(Tags);
    }
}

FDelegateHandle FRemAbilityGameplayTagEventSource::RegisterTagEvent(const FGameplayTag& Tag,
    const FEventDelegate& Delegate)
{
    // won't override an existing registration (mirrors FRemScopedAbilityTagEventManager::RegisterEventUnique)
    if (const auto* ExistingHandle = EventHandleMap.Find(Tag))
    {
        return *ExistingHandle;
    }

    const auto DelegateHandle = AbilitySystem->RegisterGameplayTagEvent(Tag,
        EGameplayTagEventType::NewOrRemoved).Add(Delegate);
    EventHandleMap.Add(Tag, DelegateHandle);

    return DelegateHandle;
}

void FRemAbilityGameplayTagEventSource::UnregisterTagEvent(const FGameplayTag& Tag)
{
    if (auto* Handle = EventHandleMap.Find(Tag))
    {
        auto bSucceed = AbilitySystem->UnregisterGameplayTagEvent(*Handle, Tag,
            EGameplayTagEventType::NewOrRemoved);
        if (!bSucceed)
        {
            bSucceed = AbilitySystem->UnregisterGameplayTagEvent(*Handle, Tag,
                EGameplayTagEventType::AnyCountChange);
        }

        EventHandleMap.Remove(Tag);
    }
}

void FRemAbilityGameplayTagEventSource::UnregisterAllTagEvents()
{
    if (!AbilitySystem)
    {
        EventHandleMap.Reset();
        return;
    }

    for (auto It = EventHandleMap.CreateIterator(); It; ++It)
    {
        auto bSucceed = AbilitySystem->UnregisterGameplayTagEvent(It.Value(), It.Key(),
            EGameplayTagEventType::NewOrRemoved);
        if (!bSucceed)
        {
            bSucceed = AbilitySystem->UnregisterGameplayTagEvent(It.Value(), It.Key(),
                EGameplayTagEventType::AnyCountChange);
        }

        It.RemoveCurrent();
    }
}

AActor* FRemAbilityGameplayTagEventSource::GetAvatarActor() const
{
    return AbilitySystem ? AbilitySystem->GetAvatarActor() : nullptr;
}

bool FRemAbilityGameplayTagEventSource::IsValid() const
{
    return ::IsValid(AbilitySystem);
}
