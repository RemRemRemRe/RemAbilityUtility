// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Containers/Map.h"
#include "Delegates/DelegateSignatureImpl.inl"
#include "UObject/WeakObjectPtrTemplates.h"

#include "RemMultiListenerAbilityTagEventManager.generated.h"

class UAbilitySystemComponent;

/**
 * multi-listener tag event manager for UAbilitySystemComponent::RegisterGameplayTagEvent
 *
 * bookkeeping is a multimap keyed by tag: several listeners may share one tag, and the handle
 * identifies the bound delegate instance rather than one registration, so unregistering takes
 * both the tag and the handle to pick which binding to remove
 *
 * for the single-binding manager (one handle per tag; registering again overrides it), see
 * FRemScopedAbilityTagEventManager in RemTagEventManager.h; the two coexist because that one
 * replaces a tag's binding while this one keeps every listener
 *
 * the ability system is held weakly, so the manager never keeps it alive; un-listen everything
 * through UnRegisterEvents or Reset
 */
USTRUCT(BlueprintType)
struct REMABILITYUTILITY_API FRemMultiListenerAbilityTagEventManager
{
    GENERATED_BODY()

    using ThisClass = FRemMultiListenerAbilityTagEventManager;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rem", meta = (AllowPrivateAccess))
    TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem{};

    TMultiMap<FGameplayTag, FDelegateHandle> TagToHandles{};

public:
    using FEventDelegate = TMulticastDelegate<void(FGameplayTag, int32)>::FDelegate;

    FRemMultiListenerAbilityTagEventManager(const ThisClass& Other)                = delete;
    FRemMultiListenerAbilityTagEventManager(ThisClass&& Other) noexcept            = delete;
    FRemMultiListenerAbilityTagEventManager& operator=(const ThisClass& Other)     = delete;
    FRemMultiListenerAbilityTagEventManager& operator=(ThisClass&& Other) noexcept = delete;
    ~FRemMultiListenerAbilityTagEventManager() noexcept                            = default;

    explicit FRemMultiListenerAbilityTagEventManager(UAbilitySystemComponent* InAbilitySystem = nullptr);

    /** @return the ability system this manager listens to, null when none is set */
    [[nodiscard]] UAbilitySystemComponent* TryGetAbilitySystem() const;

    [[nodiscard]] bool IsValid() const;

    /**
     * register a listener for a tag; the ASC broadcast fires with (Tag, Count)
     * @return the bound delegate instance's handle, invalid when the delegate is unbound or no
     * ability system is set
     */
    [[nodiscard]] FDelegateHandle RegisterEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate);

    /**
     * unregister one registration; the tag selects the binding when a single delegate instance
     * was registered for several tags
     * @return whether the ability system had a matching binding
     */
    [[nodiscard]] bool UnregisterEvent(const FGameplayTag& Tag, FDelegateHandle Handle);

    void UnRegisterEvents();

    /**
     * it will call UnRegisterEvents if the ability system changes
     * @return true if the ability system changed
     */
    [[nodiscard]] bool SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

    void Reset();
};

template <>
struct TStructOpsTypeTraits<FRemMultiListenerAbilityTagEventManager>
    : TStructOpsTypeTraitsBase2<FRemMultiListenerAbilityTagEventManager>
{
    enum
    {
        WithCopy = false
    };
};
