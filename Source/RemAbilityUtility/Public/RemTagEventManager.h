// Copyright RemRemRemRe. 2024. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Delegates/DelegateSignatureImpl.inl"
#include "Macro/RemGenerateMembersMacro.h"

#include "RemTagEventManager.generated.h"

struct FGameplayEventData;

namespace EGameplayTagEventType
{
	enum Type : int;
}

class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct REMABILITYUTILITY_API FRemAbilityEventManagerBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rem", meta = (AllowPrivateAccess, ShowInnerProperties))
	TObjectPtr<UAbilitySystemComponent> AbilitySystem{};

public:
	explicit FRemAbilityEventManagerBase(UAbilitySystemComponent* InAbilitySystem = nullptr);

	REM_DEFINE_CONST_ONLY_GETTERS_RETURN_REFERENCE_SIMPLE(AbilitySystem)

	/**
	 * @return true if AbilitySystem changed
	 */
	bool SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

	bool IsValid() const;
};

/**
 * event manager for UAbilitySystemComponent::RegisterGameplayTagEvent
 *
 * un-listen all events when out of scope
 */
USTRUCT(BlueprintType)
struct REMABILITYUTILITY_API FRemScopedAbilityTagEventManager : public FRemAbilityEventManagerBase
{
	GENERATED_BODY()

private:
	TMap<FGameplayTag, FDelegateHandle> EventHandleMap{};

public:
	using FEventDelegate = TMulticastDelegate<void(const FGameplayTag, int32)>::FDelegate;
	using Super::Super;

	~FRemScopedAbilityTagEventManager();

	REM_DEFINE_CONST_ONLY_GETTERS_RETURN_REFERENCE_SIMPLE(EventHandleMap)

	bool HasEvent(const FGameplayTag& Tag) const;

	// WILL override events if bound
	void RegisterEvent(const FGameplayTag& Tag, EGameplayTagEventType::Type EventType, const FEventDelegate& Delegate);
	void RegisterEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate);
	// won't override events if bound
	void RegisterEventUnique(const FGameplayTag& Tag, EGameplayTagEventType::Type EventType, const FEventDelegate& Delegate);
	void RegisterEventUnique(const FGameplayTag& Tag, const FEventDelegate& Delegate);

	/**
	 * it first try remove NewOrRemoved event, if failed, retry as AnyCountChange event
	 */
	bool UnRegisterEvent(const FGameplayTag& Tag);
	bool UnRegisterEvent(const FGameplayTag& Tag, EGameplayTagEventType::Type EventType);

	void Reset();

protected:
	void UnRegisterEvents();
};

/**
 * event manager for UAbilitySystemComponent::GenericGameplayEventCallbacks
 *
 * un-listen all events when out of scope
 */
USTRUCT(BlueprintType)
struct REMABILITYUTILITY_API FRemScopedAbilityGameplayEventManager : public FRemAbilityEventManagerBase
{
	GENERATED_BODY()

private:
	TMap<FGameplayTag, FDelegateHandle> EventHandleMap{};

public:
	using FEventDelegate = TMulticastDelegate<void(const FGameplayEventData*)>::FDelegate;
	using Super::Super;

	~FRemScopedAbilityGameplayEventManager();

	REM_DEFINE_CONST_ONLY_GETTERS_RETURN_REFERENCE_SIMPLE(EventHandleMap)

	bool HasEvent(const FGameplayTag& Tag) const;

	// WILL override events if bound
	void RegisterEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate);
	// won't override events if bound
	void RegisterEventUnique(const FGameplayTag& Tag, const FEventDelegate& Delegate);
	bool UnRegisterEvent(const FGameplayTag& Tag);

	void Reset();

protected:
	void UnRegisterEvents();
};
