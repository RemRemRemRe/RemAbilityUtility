// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemAbilityUtilityTestFixture.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "RemTagEventManager.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
using Rem::AbilityUtility::Private::FRemAbilityUtilityTestFixture;
}

DEFINE_SPEC(FRemScopedAbilityTagEventManagerTest, "Rem.AbilityUtility.TagEventManager",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

void FRemScopedAbilityTagEventManagerTest::Define()
{
    Describe(TEXT("RegisterEvent"), [this]
    {
        It(TEXT("should fire on tag add and remove with NewOrRemoved"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.AddRemove"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            FGameplayTag LastTag{};
            auto LastCount = 0;
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&CallCount, &LastTag, &LastCount](const FGameplayTag FiredTag, const int32 Count)
                    {
                        ++CallCount;
                        LastTag = FiredTag;
                        LastCount = Count;
                    }));

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            TestEqual(TEXT("fired on add"), CallCount, 1);
            TestEqual(TEXT("fired tag"), LastTag, Tag);
            TestEqual(TEXT("count after add"), LastCount, 1);

            Fixture.GetAbilitySystem().RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("fired on remove"), CallCount, 2);
            TestEqual(TEXT("count after remove"), LastCount, 0);
        });

        It(TEXT("should fire on every count change with AnyCountChange"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.AnyCount"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            auto LastCount = 0;
            Manager.RegisterEvent(Tag, EGameplayTagEventType::AnyCountChange,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&CallCount, &LastCount](const FGameplayTag, const int32 Count)
                    {
                        ++CallCount;
                        LastCount = Count;
                    }));

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            TestEqual(TEXT("fired on second add"), CallCount, 2);
            TestEqual(TEXT("count 2"), LastCount, 2);

            // engine behavior: non-significant removes (2 -> 1) do not broadcast AnyCountChange
            Fixture.GetAbilitySystem().RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("not fired on non-significant remove"), CallCount, 2);

            // significant remove (1 -> 0) does broadcast
            Fixture.GetAbilitySystem().RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("fired on significant remove"), CallCount, 3);
            TestEqual(TEXT("count 0"), LastCount, 0);
        });

        It(TEXT("should override a previously registered delegate for the same tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.Override"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};
            auto FirstCallCount = 0;
            auto SecondCallCount = 0;
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&FirstCallCount](const FGameplayTag, const int32)
                    {
                        ++FirstCallCount;
                    }));
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&SecondCallCount](const FGameplayTag, const int32)
                    {
                        ++SecondCallCount;
                    }));

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);

            TestEqual(TEXT("first delegate not called"), FirstCallCount, 0);
            TestEqual(TEXT("second delegate called"), SecondCallCount, 1);
            TestEqual(TEXT("single handle kept"), Manager.GetEventHandleMap().Num(), 1);
        });
    });

    Describe(TEXT("RegisterEventUnique"), [this]
    {
        It(TEXT("should ignore a second registration for the same tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.Unique"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};
            auto FirstCallCount = 0;
            auto SecondCallCount = 0;
            Manager.RegisterEventUnique(Tag,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&FirstCallCount](const FGameplayTag, const int32)
                    {
                        ++FirstCallCount;
                    }));
            Manager.RegisterEventUnique(Tag,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&SecondCallCount](const FGameplayTag, const int32)
                    {
                        ++SecondCallCount;
                    }));

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);

            TestEqual(TEXT("first delegate called"), FirstCallCount, 1);
            TestEqual(TEXT("second delegate ignored"), SecondCallCount, 0);
        });
    });

    Describe(TEXT("UnRegisterEvent"), [this]
    {
        It(TEXT("should stop firing after unregistering"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.Unregister"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayTag, const int32)
                    {
                        ++CallCount;
                    }));

            TestTrue(TEXT("unregister succeeds"), Manager.UnRegisterEvent(Tag));
            TestTrue(TEXT("no longer has event"), !Manager.HasEvent(Tag));

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            Fixture.GetAbilitySystem().RemoveLooseGameplayTag(Tag);

            TestEqual(TEXT("no callback after unregister"), CallCount, 0);
        });

        It(TEXT("should return false when the tag is not registered"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.NotBound"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};

            TestFalse(TEXT("not bound"), Manager.UnRegisterEvent(Tag));
        });

        It(TEXT("should retry with AnyCountChange when NewOrRemoved fails"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.Retry"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            Manager.RegisterEvent(Tag, EGameplayTagEventType::AnyCountChange,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayTag, const int32)
                    {
                        ++CallCount;
                    }));

            TestTrue(TEXT("unregister succeeds via fallback"), Manager.UnRegisterEvent(Tag));

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            TestEqual(TEXT("no callback after fallback unregister"), CallCount, 0);
        });
    });

    Describe(TEXT("Bulk unregister"), [this]
    {
        It(TEXT("UnRegisterEvents should clear all bindings"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.BulkA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.BulkB"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            const auto Delegate = FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                });
            Manager.RegisterEvent(TagA, Delegate);
            Manager.RegisterEvent(TagB, Delegate);

            Manager.UnRegisterEvents();

            TestEqual(TEXT("handles cleared"), Manager.GetEventHandleMap().Num(), 0);
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagB);
            TestEqual(TEXT("no callbacks"), CallCount, 0);
        });
    });

    Describe(TEXT("SetAbilitySystem"), [this]
    {
        It(TEXT("should unregister old bindings and return true on change"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* OtherActor = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* OtherAbilitySystem = NewObject<UAbilitySystemComponent>(OtherActor);
            OtherAbilitySystem->RegisterComponent();

            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.Swap"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayTag, const int32)
                    {
                        ++CallCount;
                    }));

            TestTrue(TEXT("changed"), Manager.SetAbilitySystem(OtherAbilitySystem));
            TestFalse(TEXT("unchanged second time"), Manager.SetAbilitySystem(OtherAbilitySystem));

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            TestEqual(TEXT("old ability system no longer fires"), CallCount, 0);

            // bindings are not carried over: register again on the new ability system
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayTag, const int32)
                    {
                        ++CallCount;
                    }));
            OtherAbilitySystem->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("new ability system fires after re-register"), CallCount, 1);
        });

        It(TEXT("Reset should unregister all bindings"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Tag.Reset"));
            FRemScopedAbilityTagEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayTag, const int32)
                    {
                        ++CallCount;
                    }));

            Manager.Reset();

            TestTrue(TEXT("not valid after reset"), !Manager.IsValid());
            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            TestEqual(TEXT("no callback after reset"), CallCount, 0);
        });
    });
}

#endif // WITH_DEV_AUTOMATION_TESTS
