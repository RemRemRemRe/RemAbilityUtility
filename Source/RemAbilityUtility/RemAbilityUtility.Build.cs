// Copyright RemRemRemRe, All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class RemAbilityUtility : ModuleRules
	{
		public RemAbilityUtility(ReadOnlyTargetRules target) : base(target)
		{
			ShadowVariableWarningLevel = WarningLevel.Error;
			IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
			DefaultBuildSettings = BuildSettingsVersion.Latest;
			CppStandard = CppStandardVersion.EngineDefault;
			UnsafeTypeCastWarningLevel = WarningLevel.Warning;
			
			bEnableNonInlinedGenCppWarnings = true;
			bUseUnity = false;
			
			PrivateDependencyModuleNames.AddRange(
				new[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					
					"DeveloperSettings",
					"GameplayAbilities",
					
					"RemCommon",
				}
			);
		}
	}
}