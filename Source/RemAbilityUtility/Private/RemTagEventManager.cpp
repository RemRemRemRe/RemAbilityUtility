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

bool FRemAbilityEventManagerBase::SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
	if (InAbilitySystem != AbilitySystem)
	{
		AbilitySystem = InAbilitySystem;

		return true;
	}

	return false;
}

bool FRemAbilityEventManagerBase::IsValid() const
{
	return ::IsValid(AbilitySystem);
}

FRemScopedAbilityTagEventManager::~FRemScopedAbilityTagEventManager()
{
	Reset();
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

	const auto DelegateHandle = AbilitySystem->RegisterGameplayTagEvent(Tag, EventType).Add(Delegate);

	EventHandleMap.FindOrAdd(Tag, DelegateHandle);

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
		auto bSucceed = AbilitySystem->RegisterGameplayTagEvent(Tag, EventType).Remove(*DelegateHandle);

		REM_LOG_FUNCTION(LogRemAbilityUtility, Verbose, TEXT("unregister {0} TagEvent:{1}, delegate handle:{2}, succeed?:{3}"),
					EGameplayTagEventType::NewOrRemoved, Tag, *DelegateHandle, bSucceed);

		EventHandleMap.Remove(Tag);
		return bSucceed;
	}

	return false;
}

void FRemScopedAbilityTagEventManager::Reset()
{
	UnRegisterEvents();
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
			if (bool bSucceed{AbilitySystem->RegisterGameplayTagEvent(Data.Key, EGameplayTagEventType::NewOrRemoved)
					.Remove(Data.Value)};
				!bSucceed)
			{
				bSucceed = AbilitySystem->RegisterGameplayTagEvent(Data.Key, EGameplayTagEventType::AnyCountChange).Remove(Data.Value);
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

FRemScopedAbilityGameplayEventManager::~FRemScopedAbilityGameplayEventManager()
{
	Reset();
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

	const auto DelegateHandle = AbilitySystem->GenericGameplayEventCallbacks.FindOrAdd(Tag).Add(Delegate);
	EventHandleMap.FindOrAdd(Tag, DelegateHandle);

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
	UnRegisterEvents();
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
