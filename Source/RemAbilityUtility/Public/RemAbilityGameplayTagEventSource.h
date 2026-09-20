// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Interface/RemGameplayTagEventSource.h"

#include "RemAbilityGameplayTagEventSource.generated.h"

class UAbilitySystemComponent;

/**
 * AbilitySystemComponent-backed IRemGameplayTagEventSource
 *
 * mirrors FRemScopedAbilityTagEventManager semantics:
 * NewOrRemoved tag events, unique registration, unregister-all on destruction
 *
 * lives as a component on the view target actor, so the camera's
 * TWeakInterfacePtr auto-invalidates when the actor dies
 */
UCLASS()
class REMABILITYUTILITY_API URemAbilityGameplayTagEventSource final
    : public UActorComponent, public IRemGameplayTagEventSource
{
    GENERATED_BODY()

public:
    void SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

    // IRemGameplayTagEventSource
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
