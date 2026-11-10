// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "Delegates/DelegateSignatureImpl.inl"
#include "GameplayTagContainer.h"
#include "Interface/RemTagEventSourceInterface.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "RemMultiListenerAbilityGameplayEventManager.generated.h"

class UAbilitySystemComponent;

/** one registered payload listener: the delegate to call and the binding that currently carries it */
struct FRemAbilityPayloadListener
{
    FDelegateHandle Handle{};
    TMulticastDelegate<void(FGameplayTag, const Rem::GameplayTag::FEventPayload&)>::FDelegate Delegate{};
};

/**
 * multi-listener payload event manager for UAbilitySystemComponent::GenericGameplayEventCallbacks
 *
 * bookkeeping is a multimap keyed by tag: several listeners may share one tag, and unregistering takes both
 * the tag and the handle to pick which binding to remove
 *
 * for the single-binding manager (one handle per tag; registering again overrides it), see
 * FRemScopedAbilityGameplayEventManager in RemTagEventManager.h; the two coexist because that one
 * replaces a tag's binding while this one keeps every listener
 *
 * the ability system channel carries the event data only, so each listener is bound through an adapter that
 * captures its tag and its delegate; the adapter also builds the view array the payload hands out, because the
 * event carries its data in two separate channels (owned copies and local views) while a listener sees one
 * sequence. That array lives for one callback only, which is the payload's documented lifetime
 *
 * the ability system is held weakly, so the manager never keeps it alive; a listener outlives an ability
 * system swap - SetAbilitySystem moves it to the new system instead of dropping it - so the manager keeps
 * each listener's delegate, not only its handle. UnRegisterEvents and Reset drop the listeners themselves
 */
USTRUCT(BlueprintType)
struct REMABILITYUTILITY_API FRemMultiListenerAbilityGameplayEventManager
{
    GENERATED_BODY()

    using ThisClass = FRemMultiListenerAbilityGameplayEventManager;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rem", meta = (AllowPrivateAccess))
    TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem{};

    TMultiMap<FGameplayTag, FRemAbilityPayloadListener> TagToListeners{};

public:
    using FEventDelegate = TMulticastDelegate<void(FGameplayTag, const Rem::GameplayTag::FEventPayload&)>::FDelegate;

    FRemMultiListenerAbilityGameplayEventManager(const ThisClass& Other)                = delete;
    FRemMultiListenerAbilityGameplayEventManager(ThisClass&& Other) noexcept            = delete;
    FRemMultiListenerAbilityGameplayEventManager& operator=(const ThisClass& Other)     = delete;
    FRemMultiListenerAbilityGameplayEventManager& operator=(ThisClass&& Other) noexcept = delete;
    ~FRemMultiListenerAbilityGameplayEventManager() noexcept                            = default;

    explicit FRemMultiListenerAbilityGameplayEventManager(UAbilitySystemComponent* InAbilitySystem = nullptr);

    /** @return the ability system this manager listens to, null when none is set */
    [[nodiscard]] UAbilitySystemComponent* TryGetAbilitySystem() const;

    [[nodiscard]] bool IsValid() const;

    /**
     * register a listener for a tag; the adapter fires with (Tag, Payload)
     * @return the bound delegate instance's handle, invalid when the delegate is unbound or no
     * ability system is set; the listener stays registered either way, so a later SetAbilitySystem binds it
     */
    [[nodiscard]] FDelegateHandle RegisterEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate);

    /**
     * unregister one registration; the tag selects the binding when a single delegate instance
     * was registered for several tags
     * @return whether the ability system had a matching binding
     */
    [[nodiscard]] bool UnregisterEvent(const FGameplayTag& Tag, FDelegateHandle Handle);

    /** unbind every listener and forget it */
    void UnRegisterEvents();

    /**
     * move every listener to the given ability system; the listeners are kept
     * @return true if the ability system changed
     */
    [[nodiscard]] bool SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

    /** unbind every listener, forget it and drop the ability system */
    void Reset();

private:
    [[nodiscard]] FDelegateHandle BindEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate) const;
    [[nodiscard]] bool UnbindEvent(const FGameplayTag& Tag, FDelegateHandle Handle) const;

    /** unbind every listener but keep it, so BindEvents can carry it to another ability system */
    void UnbindEvents();

    /** bind every kept listener to the current ability system */
    void BindEvents();
};

template <>
struct TStructOpsTypeTraits<FRemMultiListenerAbilityGameplayEventManager>
    : TStructOpsTypeTraitsBase2<FRemMultiListenerAbilityGameplayEventManager>
{
    enum
    {
        WithCopy = false
    };
};
