// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Interface/RemGameplayTagEventSource.h"
#include "Interface/RemGetLifetimeBoundPtr.h"

#include "RemAbilityGameplayTagEventSource.generated.h"

class UAbilitySystemComponent;

/**
 * AbilitySystemComponent-backed IRemGameplayTagEventSource
 *
 * mirrors FRemScopedAbilityTagEventManager semantics:
 * NewOrRemoved tag events, unregister-all on destruction
 *
 * lives as a component on the view target actor, so the camera's
 * TWeakInterfacePtr auto-invalidates when the actor dies
 */
UCLASS()
class REMABILITYUTILITY_API URemAbilityGameplayTagEventSource final
    : public UActorComponent, public IRemGameplayTagEventSource, public IRemGetLifetimeBoundPtr,
      public Rem::TGetLifetimeBoundPtr<URemAbilityGameplayTagEventSource>
{
    GENERATED_BODY()

private:
    struct FRegistration
    {
        FGameplayTag Tag;
        FDelegateHandle AscHandle;
    };

    TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem{};
    TMap<FDelegateHandle, FRegistration> HandleToRegistration{};

public:
    // identity of the source: the view target actor the component lives on (not the ASC avatar,
    // which may differ, e.g. an ASC on a PlayerState)
    REM_DEFINE_LIFETIME_BOUND_PTR_OWNER(GetOwner())

    void SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // IRemGameplayTagEventSource
    virtual const FGameplayTagContainer& GetOwnedTags() const override;
    virtual void AddOwnedTags(const FGameplayTagContainer& Tags) override;
    virtual void RemoveOwnedTags(const FGameplayTagContainer& Tags) override;
    virtual FDelegateHandle RegisterTagEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate) override;
    virtual void UnregisterTagEvent(FDelegateHandle Handle) override;
    virtual void UnregisterAllTagEvents() override;
};
