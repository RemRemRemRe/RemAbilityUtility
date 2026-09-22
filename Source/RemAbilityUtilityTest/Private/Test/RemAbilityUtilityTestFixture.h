// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RemNotNull.h"
#include "RemTestWorld.h"

struct FGameplayTag;

namespace Rem::AbilityUtility::Private
{
/**
 * Test fixture: a test world with an actor owning a registered
 * UAbilitySystemComponent, so tag events and gameplay events can be
 * triggered synchronously (AddLooseGameplayTag / HandleGameplayEvent).
 */
class FRemAbilityUtilityTestFixture
{
public:
    FRemAbilityUtilityTestFixture();

    UE_NONCOPYABLE(FRemAbilityUtilityTestFixture);

    /**
     * @return the test world; never null
     */
    [[nodiscard]] UWorld* GetWorld() const noexcept;

    /**
     * @return the actor owning the ability system; never null
     */
    [[nodiscard]] TNotNull<AActor*> GetActor() const noexcept;

    /**
     * @return the component owned by the test actor; never null
     */
    [[nodiscard]] TNotNull<UAbilitySystemComponent*> GetAbilitySystem() const noexcept;

    /** Registers a native gameplay tag; returns the existing tag if already registered. */
    [[nodiscard]] static FGameplayTag AddNativeTag(const FStringView TagName);

private:
    Rem::Test::FRemTestWorld World;
    TObjectPtr<AActor> Actor;
    TObjectPtr<UAbilitySystemComponent> AbilitySystem;
};
}
