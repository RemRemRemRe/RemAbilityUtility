// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemAbilityUtilityTestFixture.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/World.h"
#include "Event/RemGameplayEventContract.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interface/RemTagEventSourceInterface.h"
#include "GameplayTag/RemTagEventLifetimeBoundInterfacePtr.h"
#include "GameplayTag/RemTagEventSourceFeature.h"
#include "Misc/AutomationTest.h"
#include "RemAbilityGameplayTagEventSource.h"
#include "Struct/RemInstancedStructStatics.inl"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
using Rem::AbilityUtility::Private::FRemAbilityUtilityTestFixture;
using FTagEventDelegate     = IRemTagEventSourceInterface::FEventDelegate;
using FPayloadEventDelegate = IRemTagEventSourceInterface::FPayloadEventDelegate;

URemAbilityGameplayTagEventSource* MakeTagEventSource(const FRemAbilityUtilityTestFixture& Fixture)
{
    auto* SourceActor = Fixture.GetWorld()->SpawnActor<AActor>();
    auto* Source      = NewObject<URemAbilityGameplayTagEventSource>(SourceActor);
    Source->RegisterComponent();
    Source->SetAbilitySystem(Fixture.GetAbilitySystem());
    return Source;
}
}

DEFINE_SPEC(FRemAbilityGameplayTagEventSourceTest, "Rem.AbilityUtility.GameplayTagEventSource",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

void FRemAbilityGameplayTagEventSourceTest::Define()
{
    Describe(TEXT("RegisterTagEvent"), [this]
    {
        It(TEXT("should fire with the tag and count when the tag is added or removed"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.AddRemove"));
            auto CallCount = 0;
            FGameplayTag LastTag{};
            auto LastCount = 0;
            (void)Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&CallCount, &LastTag, &LastCount](const FGameplayTag FiredTag, const int32 Count)
                {
                    ++CallCount;
                    LastTag   = FiredTag;
                    LastCount = Count;
                }));

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("fired on add"), CallCount, 1);
            TestEqual(TEXT("fired tag"), LastTag, Tag);
            TestEqual(TEXT("count after add"), LastCount, 1);

            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("fired on remove"), CallCount, 2);
            TestEqual(TEXT("count after remove"), LastCount, 0);
        });

        It(TEXT("should not fire for a different tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source    = MakeTagEventSource(Fixture);
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.FireA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.FireB"));
            auto CallCount = 0;
            (void)Source->RegisterTagEvent(TagA, FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                }));

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagB);
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagB);

            TestEqual(TEXT("not fired for other tag"), CallCount, 0);
        });

        It(TEXT("should support multiple listeners on the same tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.MultiListener"));
            auto FirstCallCount  = 0;
            auto SecondCallCount = 0;
            (void)Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&FirstCallCount](const FGameplayTag, const int32)
                {
                    ++FirstCallCount;
                }));
            (void)Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&SecondCallCount](const FGameplayTag, const int32)
                {
                    ++SecondCallCount;
                }));

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);

            TestEqual(TEXT("first listener fired"), FirstCallCount, 1);
            TestEqual(TEXT("second listener fired"), SecondCallCount, 1);
        });

        It(TEXT("should not crash and return an invalid handle without an ability system"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* SourceActor = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* Source      = NewObject<URemAbilityGameplayTagEventSource>(SourceActor);
            Source->RegisterComponent();

            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.NoAscRegister"));
            const auto Handle = Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [](const FGameplayTag, const int32)
                {
                }));

            TestTrue(TEXT("invalid handle without ability system"), !Handle.IsValid());
        });
    });

    Describe(TEXT("UnregisterTagEvent"), [this]
    {
        It(TEXT("should stop only the unregistered binding of the tag and keep the other listener"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.HandleUnregister"));
            auto FirstCallCount    = 0;
            auto SecondCallCount   = 0;
            const auto FirstHandle = Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&FirstCallCount](const FGameplayTag, const int32)
                {
                    ++FirstCallCount;
                }));
            const auto SecondHandle = Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&SecondCallCount](const FGameplayTag, const int32)
                {
                    ++SecondCallCount;
                }));
            TestTrue(TEXT("distinct handles"), FirstHandle != SecondHandle);

            Source->UnregisterTagEvent(Tag, FirstHandle);

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(Tag);

            TestEqual(TEXT("unregistered listener does not fire"), FirstCallCount, 0);
            TestEqual(TEXT("other listener still fires"), SecondCallCount, 2);
        });

        It(TEXT("should unregister only the requested tag when three tags share a delegate instance"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source    = MakeTagEventSource(Fixture);
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.SharedDelegateA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.SharedDelegateB"));
            const auto TagC = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.SharedDelegateC"));
            auto CountA         = 0;
            auto CountB         = 0;
            auto CountC         = 0;
            // per-tag counters: a mutation that removes a handle's first entry instead of the
            // requested tag stops a different tag and turns one of these red, independent of the
            // multimap's iteration order
            const auto Delegate = FTagEventDelegate::CreateLambda(
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

            const auto HandleA = Source->RegisterTagEvent(TagA, Delegate);
            const auto HandleB = Source->RegisterTagEvent(TagB, Delegate);
            const auto HandleC = Source->RegisterTagEvent(TagC, Delegate);
            TestTrue(TEXT("one delegate instance yields one handle"), HandleA == HandleB && HandleB == HandleC);

            // unregister the middle tag: a handle-only removal would drop the first entry instead
            Source->UnregisterTagEvent(TagB, HandleB);

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagB);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagC);
            TestEqual(TEXT("the unregistered tag does not fire"), CountB, 0);
            TestEqual(TEXT("the first tag still fires"), CountA, 1);
            TestEqual(TEXT("the last tag still fires"), CountC, 1);

            Source->UnregisterTagEvent(TagA, HandleA);
            Source->UnregisterTagEvent(TagC, HandleC);

            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagB);
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagC);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagB);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagC);
            TestEqual(TEXT("no tag fires after all bindings are unregistered"), CountA + CountB + CountC, 2);
        });
    });

    Describe(TEXT("UnregisterAllTagEvents"), [this]
    {
        It(TEXT("should clear all bindings and tolerate repeated calls"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source    = MakeTagEventSource(Fixture);
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.BulkA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.BulkB"));
            auto CallCount      = 0;
            // one shared delegate instance for both tags: the handle is the same, so only a
            // multimap that keys by tag can reach both bindings on unregister-all
            const auto Delegate = FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                });
            const auto HandleA = Source->RegisterTagEvent(TagA, Delegate);
            const auto HandleB = Source->RegisterTagEvent(TagB, Delegate);
            TestTrue(TEXT("one delegate instance yields one handle"), HandleA == HandleB);

            // baseline: both registrations are bound and fire
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagB);
            TestEqual(TEXT("both bindings fire before unregister-all"), CallCount, 2);

            Source->UnregisterAllTagEvents();
            Source->UnregisterAllTagEvents();

            // drive the same tags again: unregistered bindings must stay silent
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagB);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagB);
            TestEqual(TEXT("no further callbacks after unregister-all"), CallCount, 2);
        });

        It(TEXT("should unregister all tag events when the component is destroyed"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.DestroyComponent"));
            auto CallCount = 0;
            (void)Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                }));

            Source->DestroyComponent();

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("no callback after component destroyed"), CallCount, 0);
        });

        It(TEXT("should unregister all tag events when the owner actor ends play"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.EndPlay"));
            auto CallCount = 0;
            (void)Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                }));

            Source->GetOwner()->Destroy();

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("no callback after owner actor destroyed"), CallCount, 0);
        });
    });

    Describe(TEXT("Payload events"), [this]
    {
        It(TEXT("should deliver both views and leave the tag count channel silent"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.Payload"));

            auto PayloadCallCount      = 0;
            auto CountChannelCallCount = 0;
            FGameplayTag PayloadTag{};
            FRemEventLifecycle ReceivedLifecycle{};
            FRemEventSpatial ReceivedSpatial{};
            (void)Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&CountChannelCallCount](const FGameplayTag, const int32)
                {
                    ++CountChannelCallCount;
                }));
            (void)Source->RegisterPayloadEvent(Tag, FPayloadEventDelegate::CreateLambda(
                [&](const FGameplayTag FiredTag, const Rem::GameplayTag::FEventPayload& Payload)
                {
                    ++PayloadCallCount;
                    PayloadTag = FiredTag;

                    if (const auto Lifecycle =
                            Rem::Struct::FindConstStructView<FRemEventLifecycle>(Payload.Views).Get<0>();
                        Lifecycle.IsValid())
                    {
                        ReceivedLifecycle = Lifecycle.Get();
                    }

                    if (const auto Spatial =
                            Rem::Struct::FindConstStructView<FRemEventSpatial>(Payload.Views).Get<0>();
                        Spatial.IsValid())
                    {
                        ReceivedSpatial = Spatial.Get();
                    }
                }));

            FRemEventLifecycle Lifecycle;
            Lifecycle.EventId = 7;
            Lifecycle.Phase   = ERemLifecyclePhase::Pause;
            FRemEventSpatial Spatial;
            Spatial.Location = FVector{100.0, 200.0, 300.0};
            Spatial.Normal   = FVector::UpVector;

            // the source reads the ability system's own gameplay event channel, so an engine-level dispatch
            // reaches it without the gameplay relay the test module must not depend on
            FGameplayEventData EventData;
            EventData.EventTag = Tag;
            EventData.AddStructView(Lifecycle);
            EventData.AddStructView(Spatial);

            Fixture.GetAbilitySystem()->HandleGameplayEvent(Tag, &EventData);

            TestEqual(TEXT("fired once"), PayloadCallCount, 1);
            TestEqual(TEXT("fired with the event tag"), PayloadTag, Tag);
            TestEqual(TEXT("lifecycle id"), ReceivedLifecycle.EventId, uint32{7});
            TestTrue(TEXT("lifecycle phase"), ReceivedLifecycle.Phase == ERemLifecyclePhase::Pause);
            TestTrue(TEXT("spatial location"), ReceivedSpatial.Location == Spatial.Location);
            TestTrue(TEXT("spatial normal"), ReceivedSpatial.Normal == Spatial.Normal);
            TestEqual(TEXT("the tag count channel stays silent"), CountChannelCallCount, 0);

            Fixture.GetAbilitySystem()->HandleGameplayEvent(Tag, &EventData);
            TestEqual(TEXT("a repeated dispatch reaches the same listener"), PayloadCallCount, 2);
        });

        It(TEXT("should deliver one event to both listeners of a tag and stop delivering to the unregistered one"),
            [this]
            {
                const FRemAbilityUtilityTestFixture Fixture;
                auto* Source   = MakeTagEventSource(Fixture);
                const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                    TEXT("Test.AbilityUtility.TagEventSource.PayloadMultiListener"));

                auto FirstPayloadCallCount  = 0;
                auto SecondPayloadCallCount = 0;
                const auto FirstHandle      = Source->RegisterPayloadEvent(Tag, FPayloadEventDelegate::CreateLambda(
                    [&FirstPayloadCallCount](const FGameplayTag, const Rem::GameplayTag::FEventPayload&)
                    {
                        ++FirstPayloadCallCount;
                    }));
                (void)Source->RegisterPayloadEvent(Tag, FPayloadEventDelegate::CreateLambda(
                    [&SecondPayloadCallCount](const FGameplayTag, const Rem::GameplayTag::FEventPayload&)
                    {
                        ++SecondPayloadCallCount;
                    }));

                FRemEventLifecycle Lifecycle;
                Lifecycle.EventId = 11;

                FGameplayEventData EventData;
                EventData.EventTag = Tag;
                EventData.AddStructView(Lifecycle);

                Fixture.GetAbilitySystem()->HandleGameplayEvent(Tag, &EventData);
                TestEqual(TEXT("the first listener fires once"), FirstPayloadCallCount, 1);
                TestEqual(TEXT("the second listener fires once"), SecondPayloadCallCount, 1);

                Source->UnregisterPayloadEvent(Tag, FirstHandle);

                Fixture.GetAbilitySystem()->HandleGameplayEvent(Tag, &EventData);
                TestEqual(TEXT("the unregistered listener stays silent"), FirstPayloadCallCount, 1);
                TestEqual(TEXT("the remaining listener fires again"), SecondPayloadCallCount, 2);
            });
    });

    Describe(TEXT("SetAbilitySystem"), [this]
    {
        It(TEXT("should unregister old bindings and not migrate them to a new ability system"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source = MakeTagEventSource(Fixture);

            auto* OtherActor         = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* OtherAbilitySystem = NewObject<UAbilitySystemComponent>(OtherActor);
            OtherAbilitySystem->RegisterComponent();

            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.Swap"));
            auto CallCount      = 0;
            const auto Delegate = FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                });
            (void)Source->RegisterTagEvent(Tag, Delegate);

            Source->SetAbilitySystem(OtherAbilitySystem);

            Fixture.GetAbilitySystem()->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("old ability system no longer fires"), CallCount, 0);

            // bindings are not carried over: register again on the new ability system.
            // NOTE: do not fire the tag on the new ability system before re-registering —
            // that would consume the 0 -> 1 significant transition, and a later re-fire
            // (1 -> 2) does not broadcast NewOrRemoved.
            (void)Source->RegisterTagEvent(Tag, Delegate);
            OtherAbilitySystem->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("new ability system fires after re-register"), CallCount, 1);

            OtherAbilitySystem->RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("new ability system fires on remove"), CallCount, 2);
        });

        It(TEXT("should unregister the bindings even when the same ability system is set again"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source    = MakeTagEventSource(Fixture);
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.ReSetA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.ReSetB"));
            auto CallCount      = 0;
            const auto Delegate = FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                });
            (void)Source->RegisterTagEvent(TagA, Delegate);
            (void)Source->RegisterTagEvent(TagB, Delegate);

            // baseline: both bindings are live, so each tag fires once
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagB);
            TestEqual(TEXT("both bindings fire before re-setting"), CallCount, 2);

            // this component clears every binding on every SetAbilitySystem call, even for the same
            // ability system; that differs from FRemMultiListenerAbilityTagEventManager, which clears
            // only when the ability system changes
            Source->SetAbilitySystem(Fixture.GetAbilitySystem());

            // re-trigger both tags: remove then add so the NewOrRemoved event fires again
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->RemoveLooseGameplayTag(TagB);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem()->AddLooseGameplayTag(TagB);
            TestEqual(TEXT("re-setting the same ability system clears the bindings"), CallCount, 2);
        });
    });

    Describe(TEXT("Owned tags"), [this]
    {
        It(TEXT("should reflect AddOwnedTags and RemoveOwnedTags in GetOwnedTags"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source    = MakeTagEventSource(Fixture);
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.OwnedA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.OwnedB"));

            TestEqual(TEXT("no owned tags initially"), Source->GetOwnedTags().Num(), 0);

            FGameplayTagContainer TagsToAdd;
            TagsToAdd.AddTag(TagA);
            TagsToAdd.AddTag(TagB);
            Source->AddOwnedTags(TagsToAdd);

            TestEqual(TEXT("both tags owned"), Source->GetOwnedTags().Num(), 2);
            TestTrue(TEXT("owns tag A"), Source->GetOwnedTags().HasTag(TagA));
            TestTrue(TEXT("owns tag B"), Source->GetOwnedTags().HasTag(TagB));

            FGameplayTagContainer TagsToRemove;
            TagsToRemove.AddTag(TagA);
            Source->RemoveOwnedTags(TagsToRemove);

            TestFalse(TEXT("tag A removed"), Source->GetOwnedTags().HasTag(TagA));
            TestTrue(TEXT("tag B kept"), Source->GetOwnedTags().HasTag(TagB));
        });

        It(TEXT("should add and remove owned tags given as a view"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source    = MakeTagEventSource(Fixture);
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.OwnedViewA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.OwnedViewB"));

            // the view overload is invoked on the concrete type, which compiles only because the
            // class pulls it into scope; the default implementation builds a container and forwards
            // to the container overload
            const auto TagsToAdd = TArray<FGameplayTag>{TagA, TagB};
            Source->AddOwnedTags(MakeConstArrayView(TagsToAdd));

            TestTrue(TEXT("view add owns tag A"), Source->GetOwnedTags().HasTag(TagA));
            TestTrue(TEXT("view add owns tag B"), Source->GetOwnedTags().HasTag(TagB));

            const auto TagsToRemove = TArray<FGameplayTag>{TagA};
            Source->RemoveOwnedTags(MakeConstArrayView(TagsToRemove));

            TestFalse(TEXT("view remove drops tag A"), Source->GetOwnedTags().HasTag(TagA));
            TestTrue(TEXT("view remove keeps tag B"), Source->GetOwnedTags().HasTag(TagB));
        });

        It(TEXT("should leave owned tags unchanged for an empty view"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.OwnedEmptyView"));
            FGameplayTagContainer BaselineTags{Tag};
            Source->AddOwnedTags(BaselineTags);
            TestEqual(TEXT("baseline owns one tag"), Source->GetOwnedTags().Num(), 1);

            const TConstArrayView<FGameplayTag> EmptyView{};
            Source->AddOwnedTags(EmptyView);
            TestEqual(TEXT("empty view add leaves the owned tags unchanged"), Source->GetOwnedTags().Num(), 1);

            Source->RemoveOwnedTags(EmptyView);
            TestEqual(TEXT("empty view remove leaves the owned tags unchanged"), Source->GetOwnedTags().Num(), 1);
            TestTrue(TEXT("the baseline tag is still owned"), Source->GetOwnedTags().HasTag(Tag));
        });

        It(TEXT("should return an empty container when no ability system is set"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* SourceActor = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* Source      = NewObject<URemAbilityGameplayTagEventSource>(SourceActor);
            Source->RegisterComponent();

            TestEqual(TEXT("empty owned tags without ability system"), Source->GetOwnedTags().Num(), 0);

            FGameplayTagContainer TagsToAdd;
            TagsToAdd.AddTag(
                FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.TagEventSource.NoAsc")));
            Source->AddOwnedTags(TagsToAdd);
            TestEqual(TEXT("add is a no-op without ability system"), Source->GetOwnedTags().Num(), 0);
        });
    });

    Describe(TEXT("Identity and validity"), [this]
    {
        It(TEXT("should report the owning actor as the lifetime-bound identity"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* SourceActor = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* Source      = NewObject<URemAbilityGameplayTagEventSource>(SourceActor);
            Source->RegisterComponent();
            Source->SetAbilitySystem(Fixture.GetAbilitySystem());

            const auto Handle = Source->GetLifetimeBoundObjectPtr<IRemTagEventSourceInterface>();
            TestNotNull(TEXT("identity non-null"), Handle.GetObject());
            TestTrue(TEXT("identity is the owning actor"), Handle.GetObject() == SourceActor);
            TestTrue(TEXT("the handle points at the source"), Handle.TryGet() == Source);
        });

        It(TEXT("should stay a valid handle without an ability system, and yield no owned tags"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* SourceActor = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* Source      = NewObject<URemAbilityGameplayTagEventSource>(SourceActor);
            Source->RegisterComponent();

            Source->SetAbilitySystem(nullptr);

            const auto Handle = Source->GetLifetimeBoundObjectPtr<IRemTagEventSourceInterface>();
            TestTrue(TEXT("valid while the owning actor lives"), Handle.IsValid());
            TestTrue(TEXT("no owned tags without an ability system"), Handle->GetOwnedTags().IsEmpty());

            // the ability system is a backing store, not part of the liveness
            Source->SetAbilitySystem(Fixture.GetAbilitySystem());
            TestTrue(TEXT("the ability system can still be attached afterwards"), Handle.IsValid());
        });
    });

    Describe(TEXT("Fallback provider"), [this]
    {
        // the module's own contribution is registered for its whole lifetime, so this suite needs no
        // setup and cannot be disturbed by another module's registrations

        It(TEXT("should create a source component on an actor that has an ability system"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Actor         = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* AbilitySystem = NewObject<UAbilitySystemComponent>(Actor);
            AbilitySystem->RegisterComponent();

            const auto Source = Rem::GameplayTag::TryGetLifetimeBoundInterfacePtr(Actor);
            TestTrue(TEXT("provider returns a valid source"), Source.IsValid());
            TestNotNull(TEXT("provider creates the expected component"),
                Cast<URemAbilityGameplayTagEventSource>(Source.TryGet()));
            TestTrue(TEXT("identity is the requested actor"), Source.GetObject() == Actor);

            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.ProviderCreate"));
            auto CallCount = 0;
            (void)Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                }));
            AbilitySystem->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("provider source fires tag events"), CallCount, 1);
        });

        It(TEXT("should reuse the existing source component on subsequent calls"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Actor         = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* AbilitySystem = NewObject<UAbilitySystemComponent>(Actor);
            AbilitySystem->RegisterComponent();

            const auto First  = Rem::GameplayTag::TryGetLifetimeBoundInterfacePtr(Actor);
            const auto Second = Rem::GameplayTag::TryGetLifetimeBoundInterfacePtr(Actor);

            TestTrue(TEXT("both calls return a source"), First.IsValid() && Second.IsValid());
            TestTrue(TEXT("second call reuses the same component"), First.TryGet() == Second.TryGet());
            TestTrue(TEXT("single component attached to the actor"),
                Actor->FindComponentByClass<URemAbilityGameplayTagEventSource>() == First.TryGet());
        });

        It(TEXT("should return an invalid source for an actor without an ability system"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Actor = Fixture.GetWorld()->SpawnActor<AActor>();

            const auto Source = Rem::GameplayTag::TryGetLifetimeBoundInterfacePtr(Actor);
            TestTrue(TEXT("no source without an ability system"), !Source.IsValid());
        });
    });
}

#endif // WITH_DEV_AUTOMATION_TESTS
