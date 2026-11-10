// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "RemMultiListenerAbilityGameplayEventManager.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Containers/Array.h"
#include "Macro/RemLogMacros.h"
#include "RemAbilityUtilityLog.h"
#include "StructUtils/InstancedStruct.h"
#include "StructUtils/StructView.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RemMultiListenerAbilityGameplayEventManager)

FRemMultiListenerAbilityGameplayEventManager::FRemMultiListenerAbilityGameplayEventManager(
    UAbilitySystemComponent* InAbilitySystem)
    : AbilitySystem(InAbilitySystem)
{

}

UAbilitySystemComponent* FRemMultiListenerAbilityGameplayEventManager::TryGetAbilitySystem() const
{
    return AbilitySystem.Get();
}

bool FRemMultiListenerAbilityGameplayEventManager::IsValid() const
{
    return AbilitySystem.IsValid();
}

FDelegateHandle FRemMultiListenerAbilityGameplayEventManager::RegisterEvent(const FGameplayTag& Tag,
    const FEventDelegate& Delegate)
{
    // the listener is remembered before any binding exists, so an ability system that arrives later still
    // delivers to it (@see SetAbilitySystem)
    FRemAbilityPayloadListener Listener{.Delegate = Delegate};

    if (IsValid())
    {
        Listener.Handle = BindEvent(Tag, Delegate);
    }

    const auto Handle = Listener.Handle;
    TagToListeners.Add(Tag, Listener);

    REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, "register PayloadEvent:{}, delegate handle:{}", Tag, Handle);

    return Handle;
}

bool FRemMultiListenerAbilityGameplayEventManager::UnregisterEvent(const FGameplayTag& Tag,
    const FDelegateHandle Handle)
{
    for (auto ListenerIterator = TagToListeners.CreateKeyIterator(Tag); ListenerIterator; ++ListenerIterator)
    {
        if (ListenerIterator.Value().Handle != Handle)
        {
            continue;
        }

        const bool bSucceed = UnbindEvent(Tag, Handle);
        ListenerIterator.RemoveCurrent();

        return bSucceed;
    }

    return false;
}

void FRemMultiListenerAbilityGameplayEventManager::UnRegisterEvents()
{
    UnbindEvents();

    TagToListeners.Reset();
}

bool FRemMultiListenerAbilityGameplayEventManager::SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
    if (InAbilitySystem == AbilitySystem.Get())
    {
        return false;
    }

    UnbindEvents();

    AbilitySystem = InAbilitySystem;

    BindEvents();

    return true;
}

void FRemMultiListenerAbilityGameplayEventManager::Reset()
{
    UnRegisterEvents();

    AbilitySystem = nullptr;
}

FDelegateHandle FRemMultiListenerAbilityGameplayEventManager::BindEvent(const FGameplayTag& Tag,
    const FEventDelegate& Delegate) const
{
    // the engine channel delivers the event data alone, so the tag and the listener travel as bound variables
    return AbilitySystem->GenericGameplayEventCallbacks.FindOrAdd(Tag).AddLambda(
        [Tag, Delegate](const FGameplayEventData* Payload)
        {
            TArray<FStructView, TInlineAllocator<4>> Views;

            if (Payload)
            {
                // the event keeps its data in two channels; a listener sees the owned copies first, then the local
                // views, in one sequence. A view never owns the memory it points at, so locating a datum in a const
                // payload is still a read
                Views.Reserve(Payload->Structs.Num() + Payload->StructViews.Num());

                for (const auto& Struct : Payload->Structs)
                {
                    Views.Emplace(const_cast<FInstancedStruct&>(Struct));
                }

                Views.Append(Payload->StructViews);
            }

            Delegate.ExecuteIfBound(Tag, Rem::GameplayTag::FEventPayload{.Views = Views});
        });
}

bool FRemMultiListenerAbilityGameplayEventManager::UnbindEvent(const FGameplayTag& Tag,
    const FDelegateHandle Handle) const
{
    if (!IsValid())
    {
        return false;
    }

    auto* Listeners = AbilitySystem->GenericGameplayEventCallbacks.Find(Tag);

    return Listeners
               ? Listeners->Remove(Handle)
               : false;
}

void FRemMultiListenerAbilityGameplayEventManager::UnbindEvents()
{
    for (auto& Pair : TagToListeners)
    {
        if (!Pair.Value.Handle.IsValid())
        {
            continue;
        }

        (void)UnbindEvent(Pair.Key, Pair.Value.Handle);
        Pair.Value.Handle = {};
    }
}

void FRemMultiListenerAbilityGameplayEventManager::BindEvents()
{
    if (!IsValid())
    {
        return;
    }

    for (auto& Pair : TagToListeners)
    {
        Pair.Value.Handle = BindEvent(Pair.Key, Pair.Value.Delegate);
    }
}
