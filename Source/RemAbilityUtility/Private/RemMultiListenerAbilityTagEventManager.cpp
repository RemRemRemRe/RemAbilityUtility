// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "RemMultiListenerAbilityTagEventManager.h"

#include "AbilitySystemComponent.h"
#include "RemAbilityUtilityLog.h"
#include "Macro/RemLogMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RemMultiListenerAbilityTagEventManager)

FRemMultiListenerAbilityTagEventManager::FRemMultiListenerAbilityTagEventManager(
    UAbilitySystemComponent* InAbilitySystem)
    : AbilitySystem(InAbilitySystem)
{

}

UAbilitySystemComponent* FRemMultiListenerAbilityTagEventManager::TryGetAbilitySystem() const
{
    return AbilitySystem.Get();
}

bool FRemMultiListenerAbilityTagEventManager::IsValid() const
{
    return AbilitySystem.IsValid();
}

FDelegateHandle FRemMultiListenerAbilityTagEventManager::RegisterEvent(const FGameplayTag& Tag,
    const FEventDelegate& Delegate)
{
    if (!IsValid())
    {
        // a manager without an ability system cannot register tag events; return an invalid handle
        return {};
    }

    // the ASC multicast returns the delegate instance's own handle, so registering the same
    // instance for several tags yields the same handle
    const auto Handle = AbilitySystem->RegisterGameplayTagEvent(Tag,
        EGameplayTagEventType::NewOrRemoved).Add(Delegate);

    if (!Handle.IsValid())
    {
        // an unbound delegate has no handle, so there is nothing to remember
        return Handle;
    }

    // one delegate instance may carry several tags under the same handle, so the bookkeeping is
    // a multimap and each tag keeps its own entry
    TagToHandles.Add(Tag, Handle);

    REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, "register TagEvent:{}, delegate handle:{}", Tag, Handle);

    return Handle;
}

bool FRemMultiListenerAbilityTagEventManager::UnregisterEvent(const FGameplayTag& Tag,
    const FDelegateHandle Handle)
{
    if (!IsValid())
    {
        return false;
    }

    // registrations are always NewOrRemoved, so a matching unregister succeeds; the tag selects
    // the binding when the handle is shared across several tags
    const bool bSucceed = AbilitySystem->UnregisterGameplayTagEvent(Handle, Tag,
        EGameplayTagEventType::NewOrRemoved);

    // drop the pair even on a miss: the binding is gone either way, so a stale entry would make a
    // later unregister-all touch a pair the ability system no longer knows about
    TagToHandles.RemoveSingle(Tag, Handle);

    if (!bSucceed)
    {
        REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose,
            "unregister TagEvent:{}, delegate handle:{}, succeed?:{}", Tag, Handle, bSucceed);
    }

    return bSucceed;
}

void FRemMultiListenerAbilityTagEventManager::UnRegisterEvents()
{
    if (IsValid())
    {
        for (const auto& Pair : TagToHandles)
        {
            // registrations are always NewOrRemoved, so a matching unregister succeeds
            AbilitySystem->UnregisterGameplayTagEvent(Pair.Value, Pair.Key,
                EGameplayTagEventType::NewOrRemoved);
        }
    }

    TagToHandles.Reset();
}

bool FRemMultiListenerAbilityTagEventManager::SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
    if (InAbilitySystem != AbilitySystem.Get())
    {
        UnRegisterEvents();

        AbilitySystem = InAbilitySystem;

        return true;
    }

    return false;
}

void FRemMultiListenerAbilityTagEventManager::Reset()
{
    (void)SetAbilitySystem(nullptr);
}
