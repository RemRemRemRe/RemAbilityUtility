// Copyright RemRemRemRe. 2025. All Rights Reserved.

using UnrealBuildTool;
using Rem.BuildRule;

public class RemAbilityUtility : ModuleRules
{
	public RemAbilityUtility(ReadOnlyTargetRules target) : base(target)
	{
        RemSharedModuleRules.Apply(this);

		PrivateDependencyModuleNames.AddRange(
			[
				"Core",
				"CoreUObject",
				"Engine",

				"DeveloperSettings",
				"GameplayAbilities",
				"GameplayTags",

				"RemCommon",
			]
		);
	}
}
