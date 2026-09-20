// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "RemAbilityGameplayTagEventSource.h"

#include "AbilitySystemComponent.h"

void URemAbilityGameplayTagEventSource::SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
    UnregisterAllTagEvents();

    AbilitySystem = InAbilitySystem;
}

const FGameplayTagContainer& URemAbilityGameplayTagEventSource::GetOwnedTags() const
{
    static const FGameplayTagContainer EmptyTags{};

    return AbilitySystem ? AbilitySystem->GetOwnedGameplayTags() : EmptyTags;
}

void URemAbilityGameplayTagEventSource::AddOwnedTags(const FGameplayTagContainer& Tags)
{
    if (AbilitySystem)
    {
        AbilitySystem->AddLooseGameplayTags(Tags);
    }
}

void URemAbilityGameplayTagEventSource::RemoveOwnedTags(const FGameplayTagContainer& Tags)
{
    if (AbilitySystem)
    {
        AbilitySystem->RemoveLooseGameplayTags(Tags);
    }
}

FDelegateHandle URemAbilityGameplayTagEventSource::RegisterTagEvent(const FGameplayTag& Tag,
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

void URemAbilityGameplayTagEventSource::UnregisterTagEvent(const FGameplayTag& Tag)
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

void URemAbilityGameplayTagEventSource::UnregisterAllTagEvents()
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

AActor* URemAbilityGameplayTagEventSource::GetAvatarActor() const
{
    return AbilitySystem ? AbilitySystem->GetAvatarActor() : GetOwner();
}

bool URemAbilityGameplayTagEventSource::IsValid() const
{
    return ::IsValid(AbilitySystem) && ::IsValid(GetOwner());
}
