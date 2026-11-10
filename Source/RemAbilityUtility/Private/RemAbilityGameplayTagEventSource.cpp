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
    UnbindPayloadEvents();

    (void)TagEventManager.SetAbilitySystem(InAbilitySystem);

    if (InAbilitySystem)
    {
        for (const auto& Pair : PayloadEvents)
        {
            BindPayloadEvent(Pair.Key, InAbilitySystem);
        }
    }
}

void URemAbilityGameplayTagEventSource::OnComponentDestroyed(const bool bDestroyingHierarchy)
{
    UnregisterAllTagEvents();
    UnregisterAllPayloadEvents();

    Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void URemAbilityGameplayTagEventSource::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterAllTagEvents();
    UnregisterAllPayloadEvents();

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

FDelegateHandle URemAbilityGameplayTagEventSource::RegisterPayloadEvent(const FGameplayTag& Tag,
    const FPayloadEventDelegate& Delegate)
{
    const auto Handle = PayloadEvents.FindOrAdd(Tag).Add(Delegate);

    if (!PayloadBindings.Contains(Tag))
    {
        if (auto* AbilitySystem = TagEventManager.TryGetAbilitySystem())
        {
            BindPayloadEvent(Tag, AbilitySystem);
        }
    }

    return Handle;
}

void URemAbilityGameplayTagEventSource::UnregisterPayloadEvent(const FGameplayTag& Tag,
    const FDelegateHandle Handle)
{
    auto* Listeners = PayloadEvents.Find(Tag);
    if (!Listeners)
    {
        return;
    }

    Listeners->Remove(Handle);

    if (Listeners->IsBound())
    {
        return;
    }

    PayloadEvents.Remove(Tag);

    if (FDelegateHandle BoundHandle;
        PayloadBindings.RemoveAndCopyValue(Tag, BoundHandle))
    {
        UnbindPayloadEvent(Tag, BoundHandle);
    }
}

void URemAbilityGameplayTagEventSource::UnregisterAllPayloadEvents()
{
    PayloadEvents.Reset();

    UnbindPayloadEvents();
}

void URemAbilityGameplayTagEventSource::OnAbilitySystemGameplayEvent(const FGameplayEventData* Payload,
    const FGameplayTag Tag)
{
    const auto* Listeners = PayloadEvents.Find(Tag);
    if (!Listeners || !Payload)
    {
        return;
    }

    TArray<FStructView, TInlineAllocator<4>> Views;
    Views.Reserve(Payload->Structs.Num() + Payload->StructViews.Num());

    // the payload is const, yet a view never owns the memory it points at, so locating a typed datum in it is a read
    for (const auto& Struct : Payload->Structs)
    {
        Views.Emplace(const_cast<FInstancedStruct&>(Struct));
    }

    Views.Append(Payload->StructViews);

    const FRemTagEventPayload EventPayload{.Views = Views};

    // a listener may unregister itself while it runs
    const auto ListenersCopy = *Listeners;
    ListenersCopy.Broadcast(Tag, EventPayload);
}

void URemAbilityGameplayTagEventSource::BindPayloadEvent(const FGameplayTag& Tag,
    const TNotNull<UAbilitySystemComponent*> AbilitySystem)
{
    const auto Handle = AbilitySystem->GenericGameplayEventCallbacks.FindOrAdd(Tag)
                                     .AddUObject(this, &ThisClass::OnAbilitySystemGameplayEvent, Tag);

    PayloadBindings.Add(Tag, Handle);
}

void URemAbilityGameplayTagEventSource::UnbindPayloadEvent(const FGameplayTag& Tag,
    const FDelegateHandle Handle) const
{
    auto* AbilitySystem = TagEventManager.TryGetAbilitySystem();
    if (!AbilitySystem)
    {
        return;
    }

    if (auto* Delegates = AbilitySystem->GenericGameplayEventCallbacks.Find(Tag))
    {
        Delegates->Remove(Handle);
    }
}

void URemAbilityGameplayTagEventSource::UnbindPayloadEvents()
{
    for (const auto& Pair : PayloadBindings)
    {
        UnbindPayloadEvent(Pair.Key, Pair.Value);
    }

    PayloadBindings.Reset();
}
