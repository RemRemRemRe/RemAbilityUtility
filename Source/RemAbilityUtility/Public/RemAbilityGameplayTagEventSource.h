// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "Components/RemActorComponent.h"

#include "Interface/RemTagEventSourceInterface.h"
#include "Interface/RemGetLifetimeBoundOwnerInterface.h"
#include "Interface/RemGetLifetimeBoundObjectPtrInterface.h"

#include "Containers/Map.h"
#include "Delegates/DelegateSignatureImpl.inl"
#include "RemMultiListenerAbilityTagEventManager.h"
#include "GameplayTagContainer.h"

#include "RemAbilityGameplayTagEventSource.generated.h"

struct FGameplayEventData;
class UAbilitySystemComponent;

/**
 * AbilitySystemComponent-backed IRemTagEventSourceInterface
 *
 * tag event bookkeeping is delegated to FRemMultiListenerAbilityTagEventManager: NewOrRemoved tag
 * events, unregister-all on destruction; the handle is the underlying multicast delegate instance's
 * own handle (registering one instance for several tags yields the same handle), and the tag is
 * passed back on unregister to pick which binding to remove
 *
 * lives as a component on the view target actor, so the camera's
 * Rem::GameplayTag::FLifetimeBoundInterfacePtr auto-invalidates when the actor dies
 *
 * the payload channel subscribes on the ability system's GenericGameplayEventCallbacks, which are keyed by
 * exact tag, and keeps its listeners across an ability system swap (@see IRemTagEventSourceInterface)
 */
UCLASS()
class REMABILITYUTILITY_API URemAbilityGameplayTagEventSource final
    : public URemActorComponent, public IRemTagEventSourceInterface, public IRemGetLifetimeBoundOwnerInterface,
      public Rem::TGetLifetimeBoundObjectPtr<URemAbilityGameplayTagEventSource, AActor>
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, Transient, Category = "Rem")
    FRemMultiListenerAbilityTagEventManager TagEventManager{};

    /** payload listeners per tag; these survive an ability system swap */
    TMap<FGameplayTag, TMulticastDelegate<void(FGameplayTag, const FRemTagEventPayload&)>> PayloadEvents{};

    /** the delegate each tag is subscribed with on the current ability system */
    TMap<FGameplayTag, FDelegateHandle> PayloadBindings{};

public:
    // IRemGetLifetimeBoundOwnerInterface
    [[nodiscard]] virtual UObject* GetLifetimeBoundOwner() const override;

    /**
     * point the manager at the given ability system
     *
     * tag bindings are cleared unconditionally - passing the same ability system again still drops them;
     * that is this component's existing contract and differs from FRemMultiListenerAbilityTagEventManager,
     * which clears only when the ability system changes. payload listeners are kept and rebound to the
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

private:
    /** ability system callback; the tag arrives after the payload because a multicast delegate appends the bound variables */
    void OnAbilitySystemGameplayEvent(const FGameplayEventData* Payload, FGameplayTag Tag);

    /** subscribe one tag's channel on the given ability system and remember the handle */
    void BindPayloadEvent(const FGameplayTag& Tag, TNotNull<UAbilitySystemComponent*> AbilitySystem);

    /** unsubscribe one tag's bound handle from the current ability system */
    void UnbindPayloadEvent(const FGameplayTag& Tag, FDelegateHandle Handle) const;

    /** unsubscribe every bound handle and forget them; the listeners stay registered */
    void UnbindPayloadEvents();
};
