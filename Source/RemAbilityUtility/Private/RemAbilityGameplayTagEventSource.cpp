// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "RemAbilityGameplayTagEventSource.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RemAbilityGameplayTagEventSource)

UObject* URemAbilityGameplayTagEventSource::GetLifetimeBoundOwner() const
{
    return GetOwner();
}

void URemAbilityGameplayTagEventSource::SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
    TagEventManager.UnRegisterEvents();

    (void)TagEventManager.SetAbilitySystem(InAbilitySystem);
}

void URemAbilityGameplayTagEventSource::OnComponentDestroyed(const bool bDestroyingHierarchy)
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
    if (const auto* AbilitySystem = TagEventManager.TryGetAbilitySystem())
    {
        return AbilitySystem->GetOwnedGameplayTags();
    }

    return FGameplayTagContainer::EmptyContainer;
}

void URemAbilityGameplayTagEventSource::AddOwnedTags(const FGameplayTagContainer& Tags)
{
    if (auto* AbilitySystem = TagEventManager.TryGetAbilitySystem())
    {
        AbilitySystem->AddLooseGameplayTags(Tags);
    }
}

void URemAbilityGameplayTagEventSource::RemoveOwnedTags(const FGameplayTagContainer& Tags)
{
    if (auto* AbilitySystem = TagEventManager.TryGetAbilitySystem())
    {
        AbilitySystem->RemoveLooseGameplayTags(Tags);
    }
}

FDelegateHandle URemAbilityGameplayTagEventSource::RegisterTagEvent(const FGameplayTag& Tag,
    const FEventDelegate& Delegate)
{
    return TagEventManager.RegisterEvent(Tag, Delegate);
}

void URemAbilityGameplayTagEventSource::UnregisterTagEvent(const FGameplayTag& Tag, const FDelegateHandle Handle)
{
    (void)TagEventManager.UnregisterEvent(Tag, Handle);
}

void URemAbilityGameplayTagEventSource::UnregisterAllTagEvents()
{
    TagEventManager.UnRegisterEvents();
}
