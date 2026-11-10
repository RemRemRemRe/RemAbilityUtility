// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemAbilityUtilityTestFixture.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Misc/AutomationTest.h"
#include "RemMultiListenerAbilityTagEventManager.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
using Rem::AbilityUtility::Private::FRemAbilityUtilityTestFixture;
}

DEFINE_SPEC(FRemMultiListenerAbilityTagEventManagerTest, "Rem.AbilityUtility.TagEventManager",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

void FRemMultiListenerAbilityTagEventManagerTest::Define()
{
    Describe(TEXT("Construction and validity"), [this]
    {
        It(TEXT("should be valid with an ability system and invalid without one"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const FRemMultiListenerAbilityTagEventManager DefaultManager{};
            TestFalse(TEXT("default constructed is invalid"), DefaultManager.IsValid());
            TestNull(TEXT("no ability system"), DefaultManager.TryGetAbilitySystem());

            const FRemMultiListenerAbilityTagEventManager Manager{Fixture.GetAbilitySystem()};
            TestTrue(TEXT("valid with an ability system"), Manager.IsValid());
            TestNotNull(TEXT("ability system returned"), Manager.TryGetAbilitySystem());
        });
    });

    Describe(TEXT("RegisterEvent"), [this]
    {
        It(TEXT("should return the delegate instance handle and fire the listener"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.Register"));
            FRemMultiListenerAbilityTagEventManager Manager{Fixture.GetAbilitySystem()};
            auto CallCount    = 0;
            const auto Handle = Manager.RegisterEvent(Tag,
                FRemMultiListenerAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [&CallCount](const FGameplayTag, const int32)
                    {
                        ++CallCount;
                    }));
            TestTrue(TEXT("valid handle"), Handle.IsValid());

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("the listener fires"), CallCount, 1);

            // the manager never unregisters on destruction, so drop the binding before the captured
            // counters go out of scope
            Manager.UnRegisterEvents();
        });

        It(TEXT("should return an invalid handle and remember nothing for an unbound delegate"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.Unbound"));
            FRemMultiListenerAbilityTagEventManager Manager{Fixture.GetAbilitySystem()};
            const FRemMultiListenerAbilityTagEventManager::FEventDelegate UnboundDelegate{};
            const auto Handle = Manager.RegisterEvent(Tag, UnboundDelegate);
            TestFalse(TEXT("invalid handle for an unbound delegate"), Handle.IsValid());

            // nothing was remembered, so unregistering the tag with that handle is a miss; the
            // manager logs that miss at Verbose, which the automation capture cannot assert (the
            // structured-log path only matches Warning and Error expectations)
            TestFalse(TEXT("nothing was remembered"), Manager.UnregisterEvent(Tag, Handle));

            Manager.UnRegisterEvents();
        });

        It(TEXT("should return an invalid handle without an ability system"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.NoAbilitySystem"));
            FRemMultiListenerAbilityTagEventManager Manager{};
            const auto Handle = Manager.RegisterEvent(Tag,
                FRemMultiListenerAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [](const FGameplayTag, const int32)
                    {
                    }));
            TestFalse(TEXT("invalid handle without an ability system"), Handle.IsValid());
        });
    });

    Describe(TEXT("UnregisterEvent"), [this]
    {
        It(TEXT("should remove only the requested (tag, handle) binding"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.PreciseA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.PreciseB"));
            const auto TagC = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.PreciseC"));
            auto CountA         = 0;
            auto CountB         = 0;
            auto CountC         = 0;
            // per-tag counters: a mutation that drops a shared handle's first entry instead of the
            // requested tag stops a different tag and turns one of these red, independent of the
            // multimap's iteration order
            const auto Delegate = FRemMultiListenerAbilityTagEventManager::FEventDelegate::CreateLambda(
                [&CountA, &CountB, &CountC, &TagA, &TagB, &TagC](const FGameplayTag Tag, const int32)
                {
                    if (Tag == TagA)
                    {
                        ++CountA;
                    }
                    else if (Tag == TagB)
                    {
                        ++CountB;
                    }
                    else if (Tag == TagC)
                    {
                        ++CountC;
                    }
                });
            FRemMultiListenerAbilityTagEventManager Manager{Fixture.GetAbilitySystem()};

            const auto HandleA = Manager.RegisterEvent(TagA, Delegate);
            const auto HandleB = Manager.RegisterEvent(TagB, Delegate);
            const auto HandleC = Manager.RegisterEvent(TagC, Delegate);
            TestTrue(TEXT("one delegate instance yields one handle"), HandleA == HandleB && HandleB == HandleC);

            TestTrue(TEXT("unregistering the middle tag succeeds"), Manager.UnregisterEvent(TagB, HandleB));

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagB);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagC);
            TestEqual(TEXT("the unregistered tag does not fire"), CountB, 0);
            TestEqual(TEXT("the first tag still fires"), CountA, 1);
            TestEqual(TEXT("the last tag still fires"), CountC, 1);

            Manager.UnRegisterEvents();
        });

        It(TEXT("should return false and log when the (tag, handle) pair is unknown"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.MissA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.MissB"));
            FRemMultiListenerAbilityTagEventManager Manager{Fixture.GetAbilitySystem()};
            const auto HandleA = Manager.RegisterEvent(TagA,
                FRemMultiListenerAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [](const FGameplayTag, const int32)
                    {
                    }));
            TestTrue(TEXT("bound on TagA"), HandleA.IsValid());

            // the ability system reports a miss for a (tag, handle) pair it has no binding for;
            // the manager logs that miss at Verbose, which the automation capture cannot assert
            // (the structured-log path only matches Warning and Error expectations)
            TestFalse(TEXT("mismatched tag is a miss"), Manager.UnregisterEvent(TagB, HandleA));

            Manager.UnRegisterEvents();
        });
    });

    Describe(TEXT("UnRegisterEvents"), [this]
    {
        It(TEXT("should clear every remembered binding including shared handles"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.BulkA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.BulkB"));
            auto CallCount      = 0;
            // one delegate instance for both tags: the handle is the same, so only a multimap
            // keyed by tag can reach both bindings
            const auto Delegate = FRemMultiListenerAbilityTagEventManager::FEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                });
            FRemMultiListenerAbilityTagEventManager Manager{Fixture.GetAbilitySystem()};
            const auto HandleA = Manager.RegisterEvent(TagA, Delegate);
            const auto HandleB = Manager.RegisterEvent(TagB, Delegate);
            TestTrue(TEXT("one delegate instance yields one handle"), HandleA == HandleB);

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagB);
            TestEqual(TEXT("both bindings fire before the clear"), CallCount, 2);

            Manager.UnRegisterEvents();
            Manager.UnRegisterEvents();

            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagB);
            TestEqual(TEXT("no callbacks after the clear"), CallCount, 2);
        });

        It(TEXT("should reject every operation and stay invalid without an ability system"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.NoAbilitySystemCalls"));

            FRemMultiListenerAbilityTagEventManager Manager{};
            TestFalse(TEXT("invalid without an ability system"), Manager.IsValid());

            // register and unregister both bail out on the invalid ability system: no handle, no
            // binding, no null dereference, and the manager stays invalid
            const auto Handle = Manager.RegisterEvent(Tag,
                FRemMultiListenerAbilityTagEventManager::FEventDelegate::CreateLambda(
                    [](const FGameplayTag, const int32)
                    {
                    }));
            TestFalse(TEXT("register returns an invalid handle"), Handle.IsValid());
            TestFalse(TEXT("unregister is a miss"), Manager.UnregisterEvent(Tag, Handle));

            Manager.UnRegisterEvents();
            TestFalse(TEXT("still invalid after the calls"), Manager.IsValid());
        });
    });

    Describe(TEXT("SetAbilitySystem"), [this]
    {
        It(TEXT("should return true and clear the old bindings when the ability system changes"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* OtherActor         = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* OtherAbilitySystem = NewObject<UAbilitySystemComponent>(OtherActor);
            OtherAbilitySystem->RegisterComponent();

            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.Swap"));
            auto CallCount      = 0;
            const auto Delegate = FRemMultiListenerAbilityTagEventManager::FEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                });
            FRemMultiListenerAbilityTagEventManager Manager{Fixture.GetAbilitySystem()};
            (void)Manager.RegisterEvent(Tag, Delegate);

            TestTrue(TEXT("changed"), Manager.SetAbilitySystem(OtherAbilitySystem));

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("the old ability system no longer fires"), CallCount, 0);

            // bindings are not carried over: register again on the new ability system
            (void)Manager.RegisterEvent(Tag, Delegate);
            OtherAbilitySystem->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("the new ability system fires after re-register"), CallCount, 1);

            Manager.UnRegisterEvents();
        });

        It(TEXT("should return false and keep the bindings when the same ability system is set"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.Same"));
            auto CallCount = 0;
            FRemMultiListenerAbilityTagEventManager Manager{Fixture.GetAbilitySystem()};
            (void)Manager.RegisterEvent(Tag, FRemMultiListenerAbilityTagEventManager::FEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                }));

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("the binding fires before the no-op set"), CallCount, 1);

            TestFalse(TEXT("unchanged"), Manager.SetAbilitySystem(Fixture.GetAbilitySystem()));

            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("the binding survived the no-op set"), CallCount, 2);

            Manager.UnRegisterEvents();
        });
    });

    Describe(TEXT("Reset"), [this]
    {
        It(TEXT("should behave like SetAbilitySystem(nullptr)"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventManager.Reset"));
            auto CallCount = 0;
            FRemMultiListenerAbilityTagEventManager Manager{Fixture.GetAbilitySystem()};
            (void)Manager.RegisterEvent(Tag, FRemMultiListenerAbilityTagEventManager::FEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                }));

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("the binding fires before the reset"), CallCount, 1);

            Manager.Reset();
            TestFalse(TEXT("invalid after reset"), Manager.IsValid());
            TestNull(TEXT("no ability system after reset"), Manager.TryGetAbilitySystem());

            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("no callback after reset"), CallCount, 1);
        });
    });
}

#endif // WITH_DEV_AUTOMATION_TESTS
