// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "Interface/IRemGameplayTagEventSource.h"

class UAbilitySystemComponent;

/**
 * AbilitySystemComponent-backed IRemGameplayTagEventSource
 *
 * mirrors FRemScopedAbilityTagEventManager semantics:
 * NewOrRemoved tag events, unique registration, unregister-all on destruction
 */
class REMABILITYUTILITY_API FRemAbilityGameplayTagEventSource : public IRemGameplayTagEventSource
{
public:
    explicit FRemAbilityGameplayTagEventSource(UAbilitySystemComponent* InAbilitySystem);
    ~FRemAbilityGameplayTagEventSource() override;

    const FGameplayTagContainer& GetOwnedTags() const override;
    void AddOwnedTags(const FGameplayTagContainer& Tags) override;
    void RemoveOwnedTags(const FGameplayTagContainer& Tags) override;
    FDelegateHandle RegisterTagEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate) override;
    void UnregisterTagEvent(const FGameplayTag& Tag) override;
    void UnregisterAllTagEvents() override;
    AActor* GetAvatarActor() const override;
    bool IsValid() const override;

private:
    TObjectPtr<UAbilitySystemComponent> AbilitySystem;
    TMap<FGameplayTag, FDelegateHandle> EventHandleMap;
};
