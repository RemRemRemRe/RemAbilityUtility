// Copyright RemRemRemRe. 2025. All Rights Reserved.

#include "RemAbilityUtilityModule.h"
#include "Modules/ModuleManager.h"

IRemAbilityUtilityModule& IRemAbilityUtilityModule::Get()
{
	return FModuleManager::LoadModuleChecked< IRemAbilityUtilityModule >( "RemAbilityUtility" );
}

bool IRemAbilityUtilityModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded( "RemAbilityUtility" );
}


class FRemAbilityUtilityModule final : public IRemAbilityUtilityModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FRemAbilityUtilityModule, RemAbilityUtility)

void FRemAbilityUtilityModule::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	IRemAbilityUtilityModule::StartupModule();
}

void FRemAbilityUtilityModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	IRemAbilityUtilityModule::ShutdownModule();
}
