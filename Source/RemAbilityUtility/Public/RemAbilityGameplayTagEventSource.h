// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Interface/RemTagEventSourceInterface.h"
#include "Interface/RemGetLifetimeBoundOwnerInterface.h"
#include "Interface/RemGetLifetimeBoundObjectPtrInterface.h"

#include "GameplayTagContainer.h"

#include "RemAbilityGameplayTagEventSource.generated.h"

class UAbilitySystemComponent;

/**
 * AbilitySystemComponent-backed IRemTagEventSourceInterface
 *
 * mirrors FRemScopedAbilityTagEventManager semantics:
 * NewOrRemoved tag events, unregister-all on destruction
 *
 * lives as a component on the view target actor, so the camera's
 * TWeakInterfacePtr auto-invalidates when the actor dies
 */
UCLASS()
class REMABILITYUTILITY_API URemAbilityGameplayTagEventSource final
    : public UActorComponent, public IRemTagEventSourceInterface, public IRemGetLifetimeBoundOwnerInterface,
      public Rem::TGetLifetimeBoundObjectPtr<URemAbilityGameplayTagEventSource, AActor>
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
    // IRemGetLifetimeBoundOwnerInterface
    [[nodiscard]] virtual UObject* GetLifetimeBoundOwner() const override;

    void SetAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // IRemTagEventSourceInterface
    virtual const FGameplayTagContainer& GetOwnedTags() const override;
    virtual void AddOwnedTags(const FGameplayTagContainer& Tags) override;
    virtual void RemoveOwnedTags(const FGameplayTagContainer& Tags) override;
    virtual FDelegateHandle RegisterTagEvent(const FGameplayTag& Tag, const FEventDelegate& Delegate) override;
    virtual void UnregisterTagEvent(FDelegateHandle Handle) override;
    virtual void UnregisterAllTagEvents() override;
};
