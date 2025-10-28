// Copyright RemRemRemRe. 2025. All Rights Reserved.

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

    REM_DEFINE_GETTERS_RETURN_VALUE(AbilitySystem, /* no suffix */, AbilitySystem.Get())

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

    FRemScopedAbilityTagEventManager(const FRemScopedAbilityTagEventManager& Other) = delete;
    FRemScopedAbilityTagEventManager(FRemScopedAbilityTagEventManager&& Other) noexcept = delete;
    FRemScopedAbilityTagEventManager& operator=(const FRemScopedAbilityTagEventManager& Other) = delete;
    FRemScopedAbilityTagEventManager& operator=(FRemScopedAbilityTagEventManager&& Other) noexcept = delete;
    ~FRemScopedAbilityTagEventManager() noexcept = default;

	REM_DEFINE_CONST_ONLY_GETTERS_RETURN_REFERENCE_SIMPLE(EventHandleMap)

	bool HasEvent(const FGameplayTag& Tag) const;

	// WILL override events if bound
	void RegisterEvent(const FGameplayTag& Tag, EGameplayTagEventType::Type EventType, const FEventDelegate& Delegate);
	void RegisterEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate);
	// won't override events if bound
	void RegisterEventUnique(const FGameplayTag& Tag, EGameplayTagEventType::Type EventType, const FEventDelegate& Delegate);
	void RegisterEventUnique(const FGameplayTag& Tag, const FEventDelegate& Delegate);

	/**
	 * it first tries remove NewOrRemoved event, if failed, retry as AnyCountChange event
	 */
	bool UnRegisterEvent(const FGameplayTag& Tag);
	bool UnRegisterEvent(const FGameplayTag& Tag, EGameplayTagEventType::Type EventType);

	void Reset();

	void UnRegisterEvents();

    /**
     * it will call UnRegisterEvents if AbilitySystem changes
	 * @return true if AbilitySystem changed
	 */
	bool SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);
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

    FRemScopedAbilityGameplayEventManager(const FRemScopedAbilityGameplayEventManager& Other) = delete;
    FRemScopedAbilityGameplayEventManager(FRemScopedAbilityGameplayEventManager&& Other) noexcept = delete;
    FRemScopedAbilityGameplayEventManager& operator=(const FRemScopedAbilityGameplayEventManager& Other) = delete;
    FRemScopedAbilityGameplayEventManager& operator=(FRemScopedAbilityGameplayEventManager&& Other) noexcept = delete;
    ~FRemScopedAbilityGameplayEventManager() noexcept = default;

	REM_DEFINE_CONST_ONLY_GETTERS_RETURN_REFERENCE_SIMPLE(EventHandleMap)

	bool HasEvent(const FGameplayTag& Tag) const;

	// WILL override events if bound
	void RegisterEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate);
	// won't override events if bound
	void RegisterEventUnique(const FGameplayTag& Tag, const FEventDelegate& Delegate);
	bool UnRegisterEvent(const FGameplayTag& Tag);

	void Reset();

	void UnRegisterEvents();

    /**
     * it will call UnRegisterEvents if AbilitySystem changes
	 * @return true if AbilitySystem changed
	 */
	bool SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);
};

template<>
struct TStructOpsTypeTraits<FRemScopedAbilityTagEventManager> : TStructOpsTypeTraitsBase2<FRemScopedAbilityTagEventManager>
{
    enum
    {
        WithCopy = false
    };
};

template<>
struct TStructOpsTypeTraits<FRemScopedAbilityGameplayEventManager> : TStructOpsTypeTraitsBase2<FRemScopedAbilityGameplayEventManager>
{
    enum
    {
        WithCopy = false
    };
};
