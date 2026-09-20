// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemAbilityUtilityTestFixture.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interface/RemGameplayTagEventSource.h"
#include "Interface/RemGameplayTagEventSourceHandle.h"
#include "Interface/RemGameplayTagEventSourceProvider.h"
#include "Misc/AutomationTest.h"
#include "RemAbilityGameplayTagEventSource.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
using Rem::AbilityUtility::Private::FRemAbilityUtilityTestFixture;
using FTagEventDelegate = IRemGameplayTagEventSource::FEventDelegate;

URemAbilityGameplayTagEventSource* MakeTagEventSource(const FRemAbilityUtilityTestFixture& Fixture)
{
    auto* SourceActor = Fixture.GetWorld()->SpawnActor<AActor>();
    auto* Source      = NewObject<URemAbilityGameplayTagEventSource>(SourceActor);
    Source->RegisterComponent();
    Source->SetAbilitySystem(&Fixture.GetAbilitySystem());
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
            int32 CallCount = 0;
            FGameplayTag LastTag{};
            int32 LastCount = 0;
            Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&CallCount, &LastTag, &LastCount](const FGameplayTag FiredTag, const int32 Count)
                {
                    ++CallCount;
                    LastTag   = FiredTag;
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

        It(TEXT("should not fire for a different tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source    = MakeTagEventSource(Fixture);
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.FireA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.FireB"));
            int32 CallCount = 0;
            Source->RegisterTagEvent(TagA, FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                }));

            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagB);
            Fixture.GetAbilitySystem().RemoveLooseGameplayTag(TagB);

            TestEqual(TEXT("not fired for other tag"), CallCount, 0);
        });

        It(TEXT("should support multiple listeners on the same tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.MultiListener"));
            int32 FirstCallCount  = 0;
            int32 SecondCallCount = 0;
            Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&FirstCallCount](const FGameplayTag, const int32)
                {
                    ++FirstCallCount;
                }));
            Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&SecondCallCount](const FGameplayTag, const int32)
                {
                    ++SecondCallCount;
                }));

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);

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
        It(TEXT("should stop only the unregistered handle and keep the other listener"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.HandleUnregister"));
            int32 FirstCallCount   = 0;
            int32 SecondCallCount  = 0;
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

            Source->UnregisterTagEvent(FirstHandle);

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            Fixture.GetAbilitySystem().RemoveLooseGameplayTag(Tag);

            TestEqual(TEXT("unregistered listener does not fire"), FirstCallCount, 0);
            TestEqual(TEXT("other listener still fires"), SecondCallCount, 2);
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
            int32 CallCount     = 0;
            const auto Delegate = FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                });
            // the same delegate instance is registered for two tags on purpose: UE
            // multicast delegates return the delegate instance's own handle, so both
            // registrations collide in HandleToTagMap — unregister-all must still clear
            // every binding (regression binding for the handle-collision defect)
            Source->RegisterTagEvent(TagA, Delegate);
            Source->RegisterTagEvent(TagB, Delegate);

            Source->UnregisterAllTagEvents();
            Source->UnregisterAllTagEvents();

            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagB);
            TestEqual(TEXT("no callbacks after unregister-all"), CallCount, 0);
        });

        It(TEXT("should unregister all tag events when the component is destroyed"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.DestroyComponent"));
            int32 CallCount = 0;
            Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                }));

            Source->DestroyComponent();

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            Fixture.GetAbilitySystem().RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("no callback after component destroyed"), CallCount, 0);
        });

        It(TEXT("should unregister all tag events when the owner actor ends play"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Source   = MakeTagEventSource(Fixture);
            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.EndPlay"));
            int32 CallCount = 0;
            Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                }));

            Source->GetOwner()->Destroy();

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            TestEqual(TEXT("no callback after owner actor destroyed"), CallCount, 0);
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
            int32 CallCount     = 0;
            const auto Delegate = FTagEventDelegate::CreateLambda(
                [&CallCount](const FGameplayTag, const int32)
                {
                    ++CallCount;
                });
            Source->RegisterTagEvent(Tag, Delegate);

            Source->SetAbilitySystem(OtherAbilitySystem);

            Fixture.GetAbilitySystem().AddLooseGameplayTag(Tag);
            TestEqual(TEXT("old ability system no longer fires"), CallCount, 0);

            // bindings are not carried over: register again on the new ability system.
            // NOTE: do not fire the tag on the new ability system before re-registering —
            // that would consume the 0 -> 1 significant transition, and a later re-fire
            // (1 -> 2) does not broadcast NewOrRemoved.
            Source->RegisterTagEvent(Tag, Delegate);
            OtherAbilitySystem->AddLooseGameplayTag(Tag);
            TestEqual(TEXT("new ability system fires after re-register"), CallCount, 1);

            OtherAbilitySystem->RemoveLooseGameplayTag(Tag);
            TestEqual(TEXT("new ability system fires on remove"), CallCount, 2);
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
            Source->SetAbilitySystem(&Fixture.GetAbilitySystem());

            const auto Handle = Source->GetLifetimeBoundObjectPtr<IRemGameplayTagEventSource>();
            TestNotNull(TEXT("identity non-null"), Handle.GetObject());
            TestTrue(TEXT("identity is the owning actor"), Handle.GetObject() == SourceActor);
            TestTrue(TEXT("the handle points at the source"), Handle.Get() == Source);
        });

        It(TEXT("should stay a valid handle without an ability system, and yield no owned tags"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* SourceActor = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* Source      = NewObject<URemAbilityGameplayTagEventSource>(SourceActor);
            Source->RegisterComponent();

            Source->SetAbilitySystem(nullptr);

            const auto Handle = Source->GetLifetimeBoundObjectPtr<IRemGameplayTagEventSource>();
            TestTrue(TEXT("valid while the owning actor lives"), Handle.IsValid());
            TestTrue(TEXT("no owned tags without an ability system"), Handle->GetOwnedTags().IsEmpty());

            // the ability system is a backing store, not part of the liveness
            Source->SetAbilitySystem(&Fixture.GetAbilitySystem());
            TestTrue(TEXT("the ability system can still be attached afterwards"), Handle.IsValid());
        });
    });

    Describe(TEXT("Fallback provider"), [this]
    {
        BeforeEach([this]
        {
            // restore the module-registered fallback provider: other test modules (e.g. the
            // camera suite) set and clear this global, and UE automation does not guarantee
            // cross-module order — keep this Describe self-contained
            Rem::GameplayTag::SetGameplayTagEventSourceProvider(
                [](AActor* Actor) -> Rem::GameplayTag::FTagEventSourceHandle
                {
                    auto* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
                    if (!AbilitySystem)
                    {
                        return {};
                    }

                    // reuse an existing component if already attached (avoid duplicates on re-detection)
                    if (auto* Existing = Actor->FindComponentByClass<URemAbilityGameplayTagEventSource>())
                    {
                        return Existing->GetLifetimeBoundObjectPtr<IRemGameplayTagEventSource>();
                    }

                    auto* TagEventSource = NewObject<URemAbilityGameplayTagEventSource>(Actor,
                        URemAbilityGameplayTagEventSource::StaticClass(), NAME_None, RF_Transient);
                    TagEventSource->SetAbilitySystem(AbilitySystem);
                    Actor->AddInstanceComponent(TagEventSource);
                    TagEventSource->RegisterComponent();

                    return TagEventSource->GetLifetimeBoundObjectPtr<IRemGameplayTagEventSource>();
                });
        });

        AfterEach([this]
        {
            Rem::GameplayTag::SetGameplayTagEventSourceProvider({});
        });

        It(TEXT("should create a source component on an actor that has an ability system"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Actor         = Fixture.GetWorld()->SpawnActor<AActor>();
            auto* AbilitySystem = NewObject<UAbilitySystemComponent>(Actor);
            AbilitySystem->RegisterComponent();

            const auto Source = Rem::GameplayTag::TryCreateGameplayTagEventSource(Actor);
            TestTrue(TEXT("provider returns a valid source"), Source.IsValid());
            TestNotNull(TEXT("provider creates the expected component"),
                Cast<URemAbilityGameplayTagEventSource>(Source.Get()));
            TestTrue(TEXT("identity is the requested actor"), Source.GetObject() == Actor);

            const auto Tag = FRemAbilityUtilityTestFixture::AddNativeTag(
                TEXT("Test.AbilityUtility.TagEventSource.ProviderCreate"));
            int32 CallCount = 0;
            Source->RegisterTagEvent(Tag, FTagEventDelegate::CreateLambda(
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

            const auto First  = Rem::GameplayTag::TryCreateGameplayTagEventSource(Actor);
            const auto Second = Rem::GameplayTag::TryCreateGameplayTagEventSource(Actor);

            TestTrue(TEXT("both calls return a source"), First.IsValid() && Second.IsValid());
            TestTrue(TEXT("second call reuses the same component"), First.Get() == Second.Get());
            TestTrue(TEXT("single component attached to the actor"),
                Actor->FindComponentByClass<URemAbilityGameplayTagEventSource>() == First.Get());
        });

        It(TEXT("should return an invalid source for an actor without an ability system"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            auto* Actor = Fixture.GetWorld()->SpawnActor<AActor>();

            const auto Source = Rem::GameplayTag::TryCreateGameplayTagEventSource(Actor);
            TestTrue(TEXT("no source without an ability system"), !Source.IsValid());
        });
    });
}

#endif // WITH_DEV_AUTOMATION_TESTS
