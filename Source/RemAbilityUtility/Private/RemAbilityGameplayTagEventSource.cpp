// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "RemAbilityGameplayTagEventSource.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RemAbilityGameplayTagEventSource)

UObject* URemAbilityGameplayTagEventSource::GetLifetimeBoundOwner() const
{
    return GetOwner();
}

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

    return AbilitySystem.IsValid()
               ? AbilitySystem->GetOwnedGameplayTags()
               : EmptyTags;
}

void URemAbilityGameplayTagEventSource::AddOwnedTags(const FGameplayTagContainer& Tags)
{
    if (AbilitySystem.IsValid())
    {
        AbilitySystem->AddLooseGameplayTags(Tags);
    }
}

void URemAbilityGameplayTagEventSource::RemoveOwnedTags(const FGameplayTagContainer& Tags)
{
    if (AbilitySystem.IsValid())
    {
        AbilitySystem->RemoveLooseGameplayTags(Tags);
    }
}

FDelegateHandle URemAbilityGameplayTagEventSource::RegisterTagEvent(const FGameplayTag& Tag,
    const FEventDelegate& Delegate)
{
    if (!AbilitySystem.IsValid())
    {
        // a source without an ability system cannot register tag events; return an invalid handle
        return {};
    }

    // multiple listeners per tag are allowed (asset-group and setting-value scopes may share a tag)
    const auto AscHandle = AbilitySystem->RegisterGameplayTagEvent(Tag,
        EGameplayTagEventType::NewOrRemoved).Add(Delegate);

    // the ASC multicast returns the delegate instance's own handle, which collides when the same
    // delegate object is registered multiple times; keep a per-registration handle instead
    const auto OwnHandle = FDelegateHandle{FDelegateHandle::GenerateNewHandle};
    HandleToRegistration.Add(OwnHandle, {Tag, AscHandle});

    return OwnHandle;
}

void URemAbilityGameplayTagEventSource::UnregisterTagEvent(FDelegateHandle Handle)
{
    if (!AbilitySystem.IsValid())
    {
        return;
    }

    if (const auto* Registration = HandleToRegistration.Find(Handle))
    {
        // registrations are always NewOrRemoved, so a matching unregister succeeds
        AbilitySystem->UnregisterGameplayTagEvent(Registration->AscHandle, Registration->Tag,
            EGameplayTagEventType::NewOrRemoved);

        HandleToRegistration.Remove(Handle);
    }
}

void URemAbilityGameplayTagEventSource::UnregisterAllTagEvents()
{
    if (!AbilitySystem.IsValid())
    {
        HandleToRegistration.Reset();
        return;
    }

    for (auto It = HandleToRegistration.CreateIterator(); It; ++It)
    {
        const auto& Registration = It.Value();
        // registrations are always NewOrRemoved, so a matching unregister succeeds
        AbilitySystem->UnregisterGameplayTagEvent(Registration.AscHandle, Registration.Tag,
            EGameplayTagEventType::NewOrRemoved);

        It.RemoveCurrent();
    }
}
