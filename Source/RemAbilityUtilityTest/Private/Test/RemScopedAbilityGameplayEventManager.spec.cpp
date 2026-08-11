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

DEFINE_SPEC(FRemScopedAbilityGameplayEventManagerTest, "Rem.AbilityUtility.GameplayEventManager",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

void FRemScopedAbilityGameplayEventManagerTest::Define()
{
    Describe(TEXT("RegisterEvent"), [this]
    {
        It(TEXT("should fire with the payload when the event is handled"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.Fire"));
            FRemScopedAbilityGameplayEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            const FGameplayEventData* LastPayload{};
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                    [&CallCount, &LastPayload](const FGameplayEventData* Payload)
                    {
                        ++CallCount;
                        LastPayload = Payload;
                    }));

            FGameplayEventData Payload;
            Payload.EventTag = Tag;
            Fixture.GetAbilitySystem().HandleGameplayEvent(Tag, &Payload);

            TestEqual(TEXT("fired"), CallCount, 1);
            TestTrue(TEXT("payload passed through"), LastPayload == &Payload);
        });

        It(TEXT("should not fire for a different tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.A"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.B"));
            FRemScopedAbilityGameplayEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            Manager.RegisterEvent(TagA,
                FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayEventData*)
                    {
                        ++CallCount;
                    }));

            FGameplayEventData Payload;
            Fixture.GetAbilitySystem().HandleGameplayEvent(TagB, &Payload);

            TestEqual(TEXT("not fired for other tag"), CallCount, 0);
        });

        It(TEXT("should override a previously registered delegate for the same tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.Override"));
            FRemScopedAbilityGameplayEventManager Manager{&Fixture.GetAbilitySystem()};
            auto FirstCallCount = 0;
            auto SecondCallCount = 0;
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                    [&FirstCallCount](const FGameplayEventData*)
                    {
                        ++FirstCallCount;
                    }));
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                    [&SecondCallCount](const FGameplayEventData*)
                    {
                        ++SecondCallCount;
                    }));

            FGameplayEventData Payload;
            Fixture.GetAbilitySystem().HandleGameplayEvent(Tag, &Payload);

            TestEqual(TEXT("first delegate not called"), FirstCallCount, 0);
            TestEqual(TEXT("second delegate called"), SecondCallCount, 1);
        });
    });

    Describe(TEXT("RegisterEventUnique"), [this]
    {
        It(TEXT("should ignore a second registration for the same tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.Unique"));
            FRemScopedAbilityGameplayEventManager Manager{&Fixture.GetAbilitySystem()};
            auto FirstCallCount = 0;
            auto SecondCallCount = 0;
            Manager.RegisterEventUnique(Tag,
                FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                    [&FirstCallCount](const FGameplayEventData*)
                    {
                        ++FirstCallCount;
                    }));
            Manager.RegisterEventUnique(Tag,
                FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                    [&SecondCallCount](const FGameplayEventData*)
                    {
                        ++SecondCallCount;
                    }));

            FGameplayEventData Payload;
            Fixture.GetAbilitySystem().HandleGameplayEvent(Tag, &Payload);

            TestEqual(TEXT("first delegate called"), FirstCallCount, 1);
            TestEqual(TEXT("second delegate ignored"), SecondCallCount, 0);
        });
    });

    Describe(TEXT("UnRegisterEvent"), [this]
    {
        It(TEXT("should stop firing after unregistering"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.Unregister"));
            FRemScopedAbilityGameplayEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayEventData*)
                    {
                        ++CallCount;
                    }));

            TestTrue(TEXT("unregister succeeds"), Manager.UnRegisterEvent(Tag));
            TestTrue(TEXT("no longer has event"), !Manager.HasEvent(Tag));

            FGameplayEventData Payload;
            Fixture.GetAbilitySystem().HandleGameplayEvent(Tag, &Payload);

            TestEqual(TEXT("no callback after unregister"), CallCount, 0);
        });

        It(TEXT("should return false when the tag is not registered"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.NotBound"));
            FRemScopedAbilityGameplayEventManager Manager{&Fixture.GetAbilitySystem()};

            TestFalse(TEXT("not bound"), Manager.UnRegisterEvent(Tag));
        });
    });

    Describe(TEXT("Bulk unregister"), [this]
    {
        It(TEXT("UnRegisterEvents should clear all bindings"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.BulkA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.BulkB"));
            FRemScopedAbilityGameplayEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            const auto Delegate = FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                [&CallCount](const FGameplayEventData*)
                {
                    ++CallCount;
                });
            Manager.RegisterEvent(TagA, Delegate);
            Manager.RegisterEvent(TagB, Delegate);

            Manager.UnRegisterEvents();

            TestEqual(TEXT("handles cleared"), Manager.GetEventHandleMap().Num(), 0);
            FGameplayEventData Payload;
            Fixture.GetAbilitySystem().HandleGameplayEvent(TagA, &Payload);
            Fixture.GetAbilitySystem().HandleGameplayEvent(TagB, &Payload);
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

            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Gameplay.Swap"));
            FRemScopedAbilityGameplayEventManager Manager{&Fixture.GetAbilitySystem()};
            auto CallCount = 0;
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayEventData*)
                    {
                        ++CallCount;
                    }));

            TestTrue(TEXT("changed"), Manager.SetAbilitySystem(OtherAbilitySystem));
            TestFalse(TEXT("unchanged second time"), Manager.SetAbilitySystem(OtherAbilitySystem));

            FGameplayEventData Payload;
            Fixture.GetAbilitySystem().HandleGameplayEvent(Tag, &Payload);
            TestEqual(TEXT("old ability system no longer fires"), CallCount, 0);

            // bindings are not carried over: register again on the new ability system
            Manager.RegisterEvent(Tag,
                FRemScopedAbilityGameplayEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayEventData*)
                    {
                        ++CallCount;
                    }));
            OtherAbilitySystem->HandleGameplayEvent(Tag, &Payload);
            TestEqual(TEXT("new ability system fires after re-register"), CallCount, 1);
        });
    });
}

#endif // WITH_DEV_AUTOMATION_TESTS
