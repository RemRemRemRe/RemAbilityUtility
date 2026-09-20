// Copyright RemRemRemRe. 2025. All Rights Reserved.

#include "RemAbilityUtilityModule.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Interface/RemTagEventSourceInterface.h"
#include "GameplayTag/RemTagEventLifetimeBoundInterfacePtr.h"
#include "GameplayTag/RemTagEventSourceProvider.h"
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

class FRemAbilityUtilityModule final : public IRemAbilityUtilityModule
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

}

IMPLEMENT_MODULE(FRemAbilityUtilityModule, RemAbilityUtility)

void FRemAbilityUtilityModule::StartupModule()
{
    IRemAbilityUtilityModule::StartupModule();

    // provide the ASC-backed fallback gameplay tag event source (used by RemGameplayCamera)
    Rem::GameplayTag::SetTagEventSourceProvider(
        [](const TNotNull<AActor*> NotNullActor) -> Rem::GameplayTag::FLifetimeBoundInterfacePtr
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
        });
}

void FRemAbilityUtilityModule::ShutdownModule()
{
    // clear the registered provider so the RemCommon factory doesn't point into an unloaded DLL
    Rem::GameplayTag::SetTagEventSourceProvider({});

    IRemAbilityUtilityModule::ShutdownModule();
}
