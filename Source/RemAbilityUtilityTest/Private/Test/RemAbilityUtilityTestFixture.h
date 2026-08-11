// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

    UWorld* GetWorld() const noexcept;

    UAbilitySystemComponent& GetAbilitySystem() const noexcept;

    /** Registers a native gameplay tag; returns the existing tag if already registered. */
    [[nodiscard]] static FGameplayTag AddNativeTag(const FStringView TagName);

private:
    Rem::Test::FRemTestWorld World;
    TObjectPtr<AActor> Actor;
    TObjectPtr<UAbilitySystemComponent> AbilitySystem;
};
}
