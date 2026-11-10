// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "Components/RemActorComponent.h"

#include "Interface/RemTagEventSourceInterface.h"
#include "Interface/RemGetLifetimeBoundOwnerInterface.h"
#include "Interface/RemGetLifetimeBoundObjectPtrInterface.h"

#include "GameplayTagContainer.h"
#include "RemMultiListenerAbilityGameplayEventManager.h"
#include "RemMultiListenerAbilityTagEventManager.h"

#include "RemAbilityGameplayTagEventSource.generated.h"

class UAbilitySystemComponent;

/**
 * AbilitySystemComponent-backed IRemTagEventSourceInterface
 *
 * tag event bookkeeping is delegated to FRemMultiListenerAbilityTagEventManager: NewOrRemoved tag
 * events, unregister-all on destruction; the handle is the underlying multicast delegate instance's
 * own handle (registering one instance for several tags yields the same handle), and the tag is
 * passed back on unregister to pick which binding to remove
 *
 * payload bookkeeping is delegated to FRemMultiListenerAbilityGameplayEventManager: it adapts the ability
 * system's GenericGameplayEventCallbacks, keyed by exact tag, into (tag, payload) callbacks, keeps several
 * listeners per tag and carries them across an ability system swap
 *
 * lives as a component on the view target actor, so the camera's
 * Rem::GameplayTag::FLifetimeBoundInterfacePtr auto-invalidates when the actor dies
 */
UCLASS()
class REMABILITYUTILITY_API URemAbilityGameplayTagEventSource final
    : public URemActorComponent, public IRemTagEventSourceInterface, public IRemGetLifetimeBoundOwnerInterface,
      public Rem::TGetLifetimeBoundObjectPtr<URemAbilityGameplayTagEventSource, AActor>
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, Transient, Category = "Rem")
    FRemMultiListenerAbilityTagEventManager TagEventManager{};

    UPROPERTY(VisibleAnywhere, Transient, Category = "Rem")
    FRemMultiListenerAbilityGameplayEventManager PayloadEventManager{};

public:
    // IRemGetLifetimeBoundOwnerInterface
    [[nodiscard]] virtual UObject* GetLifetimeBoundOwner() const override;

    /**
     * point the manager at the given ability system
     *
     * tag bindings are cleared unconditionally - passing the same ability system again still drops them;
     * that is this component's existing contract and differs from FRemMultiListenerAbilityTagEventManager,
     * which clears only when the ability system changes. payload listeners are kept and moved to the
     * given ability system (@see IRemTagEventSourceInterface)
     */
    void SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    // IRemTagEventSourceInterface
    using IRemTagEventSourceInterface::AddOwnedTags;
    using IRemTagEventSourceInterface::RemoveOwnedTags;

    virtual const FGameplayTagContainer& GetOwnedTags() const override;
    virtual void AddOwnedTags(const FGameplayTagContainer& Tags) override;
    virtual void RemoveOwnedTags(const FGameplayTagContainer& Tags) override;
    [[nodiscard]] virtual FDelegateHandle
    RegisterTagEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate) override;
    virtual void UnregisterTagEvent(const FGameplayTag& Tag, FDelegateHandle Handle) override;
    virtual void UnregisterAllTagEvents() override;
    [[nodiscard]] virtual FDelegateHandle
    RegisterPayloadEvent(const FGameplayTag& Tag, const FPayloadEventDelegate& Delegate) override;
    virtual void UnregisterPayloadEvent(const FGameplayTag& Tag, FDelegateHandle Handle) override;
    virtual void UnregisterAllPayloadEvents() override;
};
