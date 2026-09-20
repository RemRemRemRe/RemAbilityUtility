// Copyright RemRemRemRe. 2025. All Rights Reserved.

#include "RemAbilityUtilityModule.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Actor.h"
#include "Interface/RemTagEventSourceInterface.h"
#include "GameplayTag/RemTagEventLifetimeBoundInterfacePtr.h"
#include "GameplayTag/RemTagEventSourceFeature.h"
#include "Modules/ModuleManager.h"
#include "RemAbilityGameplayTagEventSource.h"

IRemAbilityUtilityModule& IRemAbilityUtilityModule::Get()
{
    return FModuleManager::LoadModuleChecked<IRemAbilityUtilityModule>("RemAbilityUtility");
}

bool IRemAbilityUtilityModule::IsAvailable()
{
    return FModuleManager::Get().IsModuleLoaded("RemAbilityUtility");
}

namespace
{
/** this module's contribution to the tag event source family (used by RemGameplayCamera) */
Rem::GameplayTag::FTagEventSourceFeature::FContribution MakeTagEventSourceContribution()
{
    return {
        AActor::StaticClass(),
        [](const Rem::GameplayTag::FTagEventSourceFeature::FRequest& NotNullActor)
        -> Rem::GameplayTag::FLifetimeBoundInterfacePtr
        {
            AActor* Actor = NotNullActor;

            auto* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
            if (!AbilitySystem)
            {
                return {};
            }

            // reuse an existing component if already attached (avoid duplicates on re-detection)
            if (auto* Existing = Actor->FindComponentByClass<URemAbilityGameplayTagEventSource>())
            {
                return Existing->GetLifetimeBoundObjectPtr<IRemTagEventSourceInterface>();
            }

            auto* TagEventSource = NewObject<URemAbilityGameplayTagEventSource>(Actor,
                URemAbilityGameplayTagEventSource::StaticClass(), NAME_None, RF_Transient);
            TagEventSource->SetAbilitySystem(AbilitySystem);
            Actor->AddInstanceComponent(TagEventSource);
            TagEventSource->RegisterComponent();

            return TagEventSource->GetLifetimeBoundObjectPtr<IRemTagEventSourceInterface>();
        },
    };
}

class FRemAbilityUtilityModule final : public IRemAbilityUtilityModule
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    /**
     * This module's registration in the tag event source family: it registers while this module object
     * is alive and clears when it is destroyed, so the family never reaches into an unloaded DLL.
     * ShutdownModule resets it explicitly as well; Reset() is idempotent.
     */
    Rem::GameplayTag::FTagEventSourceRegistration TagEventSourceRegistration{MakeTagEventSourceContribution()};
};
} // namespace

IMPLEMENT_MODULE(FRemAbilityUtilityModule, RemAbilityUtility)

void FRemAbilityUtilityModule::StartupModule()
{
    IRemAbilityUtilityModule::StartupModule();
}

void FRemAbilityUtilityModule::ShutdownModule()
{
    // clear the registration so the RemCommon family doesn't call into an unloaded DLL
    TagEventSourceRegistration.Reset();

    IRemAbilityUtilityModule::ShutdownModule();
}
