// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemAbilityUtilityTestFixture.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "RemTestGameplayTags.h"

namespace Rem::AbilityUtility::Private
{
FRemAbilityUtilityTestFixture::FRemAbilityUtilityTestFixture()
{
    Actor = World->SpawnActor<AActor>();
    check(Actor);

    AbilitySystem = NewObject<UAbilitySystemComponent>(Actor);
    check(AbilitySystem);
    AbilitySystem->RegisterComponent();
}

UWorld* FRemAbilityUtilityTestFixture::GetWorld() const noexcept
{
    return World;
}

UAbilitySystemComponent& FRemAbilityUtilityTestFixture::GetAbilitySystem() const noexcept
{
    return *AbilitySystem;
}

FGameplayTag FRemAbilityUtilityTestFixture::AddNativeTag(const FStringView TagName)
{
    return Rem::Test::AddNativeTag(FName{TagName});
}
}
