// Copyright RemRemRemRe. 2024. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Delegates/DelegateSignatureImpl.inl"
#include "Macro/RemGenerateMembersMacro.h"

#include "RemTagEventManager.generated.h"

namespace EGameplayTagEventType
{
	enum Type : int;
}

class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct REMABILITYUTILITY_API FRemTagEventManager
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rem", meta = (AllowPrivateAccess))
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem{};

	TMap<FGameplayTag, FDelegateHandle> TagToEventHandle{};

public:
	explicit FRemTagEventManager(UAbilitySystemComponent* InAbilitySystem = nullptr);

	~FRemTagEventManager();

	using FDelegate = TMulticastDelegate<void(const FGameplayTag, int32)>::FDelegate;

	/**
	 * @return true if AbilitySystem changed
	 */
	bool SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);
	REM_DEFINE_CONST_ONLY_GETTERS_RETURN_REFERENCE_SIMPLE(AbilitySystem)
	REM_DEFINE_CONST_ONLY_GETTERS_RETURN_REFERENCE_SIMPLE(TagToEventHandle)

	// won't override events if bound
	void RegisterGameplayTagEventUnique(FGameplayTag Tag, EGameplayTagEventType::Type EventType, const FDelegate& Delegate);
	void RegisterGameplayTagEventUnique(FGameplayTag Tag, const FDelegate& Delegate);

	// will override events if bound
	void RegisterGameplayTagEvent(FGameplayTag Tag, EGameplayTagEventType::Type EventType, const FDelegate& Delegate);
	void RegisterGameplayTagEvent(FGameplayTag Tag, const FDelegate& Delegate);

	void Reset();

	bool IsValid() const;

protected:
	void UnRegisterGameplayTagEvents();
};
