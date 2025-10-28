// Copyright RemRemRemRe. 2025. All Rights Reserved.


#include "RemTagEventManager.h"

#include "AbilitySystemComponent.h"
#include "RemAbilityUtilityLog.h"
#include "Macro/RemAssertionMacros.h"
#include "Macro/RemLogMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RemTagEventManager)

FRemAbilityEventManagerBase::FRemAbilityEventManagerBase(UAbilitySystemComponent* InAbilitySystem)
	: AbilitySystem(InAbilitySystem)
{

}

bool FRemAbilityEventManagerBase::IsValid() const
{
	return ::IsValid(AbilitySystem);
}

bool FRemScopedAbilityTagEventManager::HasEvent(const FGameplayTag& Tag) const
{
	return EventHandleMap.Contains(Tag);
}

void FRemScopedAbilityTagEventManager::RegisterEvent(const FGameplayTag& Tag, const EGameplayTagEventType::Type EventType,
	const FEventDelegate& Delegate)
{
#if REM_WITH_DEVELOPMENT_ONLY_CODE

	RemCheckVariable(AbilitySystem, return;);

#endif

	auto* ExistingHandle = EventHandleMap.Find(Tag);
	if (ExistingHandle)
	{
		AbilitySystem->UnregisterGameplayTagEvent(*ExistingHandle, Tag, EventType);

		REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("{0} TagEvent:{1}, already registerred with delegate handle:{2}, removed"),
			EventType, Tag, *ExistingHandle);
	}

	const auto DelegateHandle = AbilitySystem->RegisterGameplayTagEvent(Tag, EventType).Add(Delegate);

	if (!ExistingHandle)
	{
		EventHandleMap.Add(Tag, DelegateHandle);
	}
	else
	{
		*ExistingHandle = DelegateHandle;
	}

	REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("register {0} TagEvent:{1}, delegate handle:{2}"), EventType, Tag, DelegateHandle);
}

void FRemScopedAbilityTagEventManager::RegisterEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate)
{
	return RegisterEvent(Tag, EGameplayTagEventType::NewOrRemoved, Delegate);
}

void FRemScopedAbilityTagEventManager::RegisterEventUnique(const FGameplayTag& Tag, const EGameplayTagEventType::Type EventType,
	const FEventDelegate& Delegate)
{
	if (!HasEvent(Tag))
	{
		RegisterEvent(Tag, EventType, Delegate);
	}
	else
	{
		REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("trying of register {0} TagEvent:{1}, ignored, previous delegate handle:{2}"),
			EventType, Tag, EventHandleMap.FindChecked(Tag));
	}
}

void FRemScopedAbilityTagEventManager::RegisterEventUnique(const FGameplayTag& Tag, const FEventDelegate& Delegate)
{
	return RegisterEventUnique(Tag, EGameplayTagEventType::NewOrRemoved, Delegate);
}

bool FRemScopedAbilityTagEventManager::UnRegisterEvent(const FGameplayTag& Tag)
{
	return UnRegisterEvent(Tag, EGameplayTagEventType::NewOrRemoved)
		|| UnRegisterEvent(Tag, EGameplayTagEventType::AnyCountChange);
}

bool FRemScopedAbilityTagEventManager::UnRegisterEvent(const FGameplayTag& Tag, const EGameplayTagEventType::Type EventType)
{
#if REM_WITH_DEVELOPMENT_ONLY_CODE

	RemCheckVariable(AbilitySystem, return false;);

#endif

	if (const auto* DelegateHandle = EventHandleMap.Find(Tag))
	{
		auto bSucceed = AbilitySystem->UnregisterGameplayTagEvent(*DelegateHandle, Tag, EventType);

		REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("unregister {0} TagEvent:{1}, delegate handle:{2}, succeed?:{3}"),
					EGameplayTagEventType::NewOrRemoved, Tag, *DelegateHandle, bSucceed);

		EventHandleMap.Remove(Tag);
		return bSucceed;
	}

	return false;
}

void FRemScopedAbilityTagEventManager::Reset()
{
	SetAbilitySystem(nullptr);
}

void FRemScopedAbilityTagEventManager::UnRegisterEvents()
{
	if (IsValid())
	{
		for (const auto& Data : EventHandleMap)
		{
			auto& Tag = Data.Key;
			auto& DelegateHandle = Data.Value;
			if (bool bSucceed{AbilitySystem->UnregisterGameplayTagEvent(Data.Value, Data.Key, EGameplayTagEventType::NewOrRemoved)};
				!bSucceed)
			{
				bSucceed = AbilitySystem->UnregisterGameplayTagEvent(Data.Value, Data.Key, EGameplayTagEventType::AnyCountChange);

				REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("unregister {0} TagEvent:{1}, delegate handle:{2}, succeed?:{3}"),
					EGameplayTagEventType::AnyCountChange, Tag, DelegateHandle, bSucceed);
			}
			else
			{
				REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("unregister {0} TagEvent:{1}, delegate handle:{2}, succeed?:{3}"),
					EGameplayTagEventType::NewOrRemoved, Tag, DelegateHandle, bSucceed);
			}
		}
	}

	EventHandleMap.Reset();
	REM_LOG_FUNCTION(LogRemAbilityUtility, Log, TEXT("TagEventHandleMap reseted"));
}

bool FRemScopedAbilityTagEventManager::SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
    if (InAbilitySystem != AbilitySystem)
    {
        UnRegisterEvents();
        
        AbilitySystem = InAbilitySystem;

        return true;
    }

    return false;
}

bool FRemScopedAbilityGameplayEventManager::HasEvent(const FGameplayTag& Tag) const
{
	return EventHandleMap.Contains(Tag);
}

void FRemScopedAbilityGameplayEventManager::RegisterEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate)
{
#if REM_WITH_DEVELOPMENT_ONLY_CODE

	RemCheckVariable(AbilitySystem, return;);

#endif

	auto* ExistingHandle = EventHandleMap.Find(Tag);
	if (ExistingHandle)
	{
		if (auto* ExistedDelegate = AbilitySystem->GenericGameplayEventCallbacks.Find(Tag))
		{
			ExistedDelegate->Remove(*ExistingHandle);

			REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("TagEvent:{0}, already registerred with delegate handle:{1}, removed"),
				Tag, *ExistingHandle);
		}
		else
		{
			REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("TagEvent:{0}, already registerred with delegate handle:{1}, but ExistedDelegate not found"),
				Tag, *ExistingHandle);
		}
	}

	const auto DelegateHandle = AbilitySystem->GenericGameplayEventCallbacks.FindOrAdd(Tag).Add(Delegate);

	if (!ExistingHandle)
	{
		EventHandleMap.FindOrAdd(Tag, DelegateHandle);
	}
	else
	{
		*ExistingHandle = DelegateHandle;
	}

	REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("register GenericGameplayEvent:{0}, delegate handle:{1}"), Tag, DelegateHandle);
}

void FRemScopedAbilityGameplayEventManager::RegisterEventUnique(const FGameplayTag& Tag, const FEventDelegate& Delegate)
{
	if (!HasEvent(Tag))
	{
		RegisterEvent(Tag, Delegate);
	}
	else
	{
		REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("trying of register GenericGameplayEvent:{0}, ignored, previous delegate handle:{1}"),
			Tag, EventHandleMap.FindChecked(Tag));
	}
}

bool FRemScopedAbilityGameplayEventManager::UnRegisterEvent(const FGameplayTag& Tag)
{
#if REM_WITH_DEVELOPMENT_ONLY_CODE

	RemCheckVariable(AbilitySystem, return false;);

#endif

	if (const auto* DelegateHandle = EventHandleMap.Find(Tag))
	{
		auto* Delegate = AbilitySystem->GenericGameplayEventCallbacks.Find(Tag);
		bool bSucceed{Delegate ? Delegate->Remove(*DelegateHandle) : false};

		REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("unregister GenericGameplayEvent:{0}, delegate handle:{1}, succeed?:{2}"),
			Tag, *DelegateHandle, bSucceed);

		EventHandleMap.Remove(Tag);
		return bSucceed;
	}

	return false;
}

void FRemScopedAbilityGameplayEventManager::Reset()
{
	SetAbilitySystem(nullptr);
}

void FRemScopedAbilityGameplayEventManager::UnRegisterEvents()
{
	if (IsValid())
	{
		for (const auto& Data : EventHandleMap)
		{
			auto& Tag = Data.Key;
			auto& DelegateHandle = Data.Value;
			auto* Delegate = AbilitySystem->GenericGameplayEventCallbacks.Find(Tag);
			bool bSucceed{Delegate ? Delegate->Remove(DelegateHandle) : false};

			REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("unregister GenericGameplayEvent:{0}, delegate handle:{1}, succeed?:{2}"),
				Tag, DelegateHandle, bSucceed);
		}
	}

	EventHandleMap.Reset();
	REM_LOG_FUNCTION(LogRemAbilityUtility, Log, TEXT("GenericGameplayEventHandleMap reseted"));
}

bool FRemScopedAbilityGameplayEventManager::SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
    if (InAbilitySystem != AbilitySystem)
    {
        UnRegisterEvents();
        
        AbilitySystem = InAbilitySystem;

        return true;
    }

    return false;
}
