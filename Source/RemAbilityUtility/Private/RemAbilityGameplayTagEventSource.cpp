// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "RemAbilityGameplayTagEventSource.h"

#include "AbilitySystemComponent.h"

void URemAbilityGameplayTagEventSource::SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
    UnregisterAllTagEvents();

    AbilitySystem = InAbilitySystem;
}

void URemAbilityGameplayTagEventSource::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    UnregisterAllTagEvents();

    Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void URemAbilityGameplayTagEventSource::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterAllTagEvents();

    Super::EndPlay(EndPlayReason);
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
    // multiple listeners per tag are allowed (asset-group and setting-value scopes may share a tag);
    // the handle is the per-registration ASC multicast handle
    const auto DelegateHandle = AbilitySystem->RegisterGameplayTagEvent(Tag,
        EGameplayTagEventType::NewOrRemoved).Add(Delegate);
    HandleToTagMap.Add(DelegateHandle, Tag);

    return DelegateHandle;
}

void URemAbilityGameplayTagEventSource::UnregisterTagEvent(FDelegateHandle Handle)
{
    if (const auto* Tag = HandleToTagMap.Find(Handle))
    {
        auto bSucceed = AbilitySystem->UnregisterGameplayTagEvent(Handle, *Tag,
            EGameplayTagEventType::NewOrRemoved);
        if (!bSucceed)
        {
            bSucceed = AbilitySystem->UnregisterGameplayTagEvent(Handle, *Tag,
                EGameplayTagEventType::AnyCountChange);
        }

        HandleToTagMap.Remove(Handle);
    }
}

void URemAbilityGameplayTagEventSource::UnregisterAllTagEvents()
{
    if (!AbilitySystem)
    {
        HandleToTagMap.Reset();
        return;
    }

    for (auto It = HandleToTagMap.CreateIterator(); It; ++It)
    {
        auto bSucceed = AbilitySystem->UnregisterGameplayTagEvent(It.Key(), It.Value(),
            EGameplayTagEventType::NewOrRemoved);
        if (!bSucceed)
        {
            bSucceed = AbilitySystem->UnregisterGameplayTagEvent(It.Key(), It.Value(),
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
