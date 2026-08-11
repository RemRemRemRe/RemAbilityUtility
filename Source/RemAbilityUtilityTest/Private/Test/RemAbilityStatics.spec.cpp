// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemAbilityUtilityTestFixture.h"

#include "AbilitySystemComponent.h"
#include "GameplayTag/RemGameplayTagArray.h"
#include "GameplayTagsManager.h"
#include "Misc/AutomationTest.h"
#include "RemAbilityUtilityStatics.h"
#include "RemAbilityUtilityStatics.inl"
#include "RemNotNull.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
using Rem::AbilityUtility::Private::FRemAbilityUtilityTestFixture;
using Rem::GameplayTag::HasMatchingGameplayTags;
using Rem::Enum::ELogicOperator;
using Rem::Ability::HasAnyMatchingTags;
using Rem::Ability::HasAllMatchingTags;
using Rem::Ability::HasAnyMatchingQueryTag;
using Rem::Ability::HasAllMatchingQueryTag;
}

DEFINE_SPEC(FRemAbilityStaticsTest, "Rem.AbilityUtility.Statics",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

void FRemAbilityStaticsTest::Define()
{
    Describe(TEXT("HasMatchingGameplayTags with a container"), [this]
    {
        It(TEXT("All should require every tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.A"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.B"));
            const Rem::TNotNull<UAbilitySystemComponent*> AbilitySystem{&Fixture.GetAbilitySystem()};
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);

            FGameplayTagContainer Matching;
            Matching.AddTag(TagA);
            FGameplayTagContainer Extra;
            Extra.AddTag(TagA);
            Extra.AddTag(TagB);

            TestTrue(TEXT("all match"), HasMatchingGameplayTags<ELogicOperator::All>(AbilitySystem, Matching));
            TestFalse(TEXT("not all match"), HasMatchingGameplayTags<ELogicOperator::All>(AbilitySystem, Extra));
        });

        It(TEXT("Any should require at least one tag"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.C"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.D"));
            const Rem::TNotNull<UAbilitySystemComponent*> AbilitySystem{&Fixture.GetAbilitySystem()};
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);

            FGameplayTagContainer WithA;
            WithA.AddTag(TagA);
            FGameplayTagContainer OnlyB;
            OnlyB.AddTag(TagB);

            TestTrue(TEXT("any match"), HasMatchingGameplayTags<ELogicOperator::Any>(AbilitySystem, WithA));
            TestFalse(TEXT("no match"), HasMatchingGameplayTags<ELogicOperator::Any>(AbilitySystem, OnlyB));
        });

        It(TEXT("None should negate the any match"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.E"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.F"));
            const Rem::TNotNull<UAbilitySystemComponent*> AbilitySystem{&Fixture.GetAbilitySystem()};
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);

            FGameplayTagContainer WithA;
            WithA.AddTag(TagA);
            FGameplayTagContainer OnlyB;
            OnlyB.AddTag(TagB);

            TestFalse(TEXT("owned tag breaks none"), HasMatchingGameplayTags<ELogicOperator::None>(AbilitySystem, WithA));
            TestTrue(TEXT("unowned tag keeps none"), HasMatchingGameplayTags<ELogicOperator::None>(AbilitySystem, OnlyB));
        });
    });

    Describe(TEXT("HasMatchingGameplayTags with a single tag"), [this]
    {
        It(TEXT("All and Any should match the owned tag, None should negate"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.SingleA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.SingleB"));
            const Rem::TNotNull<UAbilitySystemComponent*> AbilitySystem{&Fixture.GetAbilitySystem()};
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);

            TestTrue(TEXT("all match"), HasMatchingGameplayTags<ELogicOperator::All>(AbilitySystem, TagA));
            TestFalse(TEXT("all not match"), HasMatchingGameplayTags<ELogicOperator::All>(AbilitySystem, TagB));
            TestTrue(TEXT("any match"), HasMatchingGameplayTags<ELogicOperator::Any>(AbilitySystem, TagA));
            TestFalse(TEXT("none not match"), HasMatchingGameplayTags<ELogicOperator::None>(AbilitySystem, TagA));
            TestTrue(TEXT("none match"), HasMatchingGameplayTags<ELogicOperator::None>(AbilitySystem, TagB));
        });
    });

    Describe(TEXT("HasMatchingGameplayTags with a tag array"), [this]
    {
        It(TEXT("should behave like the container overload"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.ArrayA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.ArrayB"));
            const Rem::TNotNull<UAbilitySystemComponent*> AbilitySystem{&Fixture.GetAbilitySystem()};
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);

            FRemGameplayTagArray WithA;
            WithA.Tags.Add(TagA);
            FRemGameplayTagArray WithB;
            WithB.Tags.Add(TagB);

            TestTrue(TEXT("any match"), HasMatchingGameplayTags<ELogicOperator::Any>(AbilitySystem, WithA));
            TestFalse(TEXT("no match"), HasMatchingGameplayTags<ELogicOperator::Any>(AbilitySystem, WithB));
        });
    });

    Describe(TEXT("HasMatchingGameplayTags with a query"), [this]
    {
        It(TEXT("should use query semantics and ignore the match mode"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.QueryA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.QueryB"));
            const Rem::TNotNull<UAbilitySystemComponent*> AbilitySystem{&Fixture.GetAbilitySystem()};
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);

            FGameplayTagContainer QueryTags;
            QueryTags.AddTag(TagA);
            QueryTags.AddTag(TagB);
            const auto Query = FGameplayTagQuery::MakeQuery_MatchAllTags(QueryTags);

            // the query overload always evaluates the query itself, ignoring All/Any/None
            TestFalse(TEXT("all mode uses query"), HasMatchingGameplayTags<ELogicOperator::All>(AbilitySystem, Query));
            TestFalse(TEXT("any mode also uses query"), HasMatchingGameplayTags<ELogicOperator::Any>(AbilitySystem, Query));
            TestFalse(TEXT("none mode also uses query"), HasMatchingGameplayTags<ELogicOperator::None>(AbilitySystem, Query));

            FGameplayTagContainer SingleTag;
            SingleTag.AddTag(TagA);
            const auto SingleQuery = FGameplayTagQuery::MakeQuery_MatchAllTags(SingleTag);
            TestTrue(TEXT("query satisfied"), HasMatchingGameplayTags<ELogicOperator::All>(AbilitySystem, SingleQuery));
        });
    });

    Describe(TEXT("Non-template statics"), [this]
    {
        It(TEXT("HasAnyMatchingTags and HasAllMatchingTags should check tag counts"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.NonTemplateA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.NonTemplateB"));
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);

            TestTrue(TEXT("any with A"), HasAnyMatchingTags(Fixture.GetAbilitySystem(), {TagA}));
            TestFalse(TEXT("any with B"), HasAnyMatchingTags(Fixture.GetAbilitySystem(), {TagB}));
            TestTrue(TEXT("all with A"), HasAllMatchingTags(Fixture.GetAbilitySystem(), {TagA}));
            TestFalse(TEXT("all with A and B"), HasAllMatchingTags(Fixture.GetAbilitySystem(), {TagA, TagB}));
        });

        It(TEXT("should treat empty tag lists as matching"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;

            TestTrue(TEXT("any with empty"), HasAnyMatchingTags(Fixture.GetAbilitySystem(), {}));
            TestTrue(TEXT("all with empty"), HasAllMatchingTags(Fixture.GetAbilitySystem(), {}));
        });

        It(TEXT("query variants should match the query tags by count"), [this]
        {
            FRemAbilityUtilityTestFixture Fixture;
            const auto TagA = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.QueryStaticsA"));
            const auto TagB = FRemAbilityUtilityTestFixture::AddNativeTag(TEXT("Test.AbilityUtility.Statics.QueryStaticsB"));
            Fixture.GetAbilitySystem().AddLooseGameplayTag(TagA);

            FGameplayTagContainer QueryTags;
            QueryTags.AddTag(TagA);
            QueryTags.AddTag(TagB);
            const auto Query = FGameplayTagQuery::MakeQuery_MatchAllTags(QueryTags);

            // these walk the query's tag array instead of evaluating the query
            TestTrue(TEXT("any in query"), HasAnyMatchingQueryTag(Fixture.GetAbilitySystem(), Query));
            TestFalse(TEXT("all in query"), HasAllMatchingQueryTag(Fixture.GetAbilitySystem(), Query));
        });
    });
}

#endif // WITH_DEV_AUTOMATION_TESTS
