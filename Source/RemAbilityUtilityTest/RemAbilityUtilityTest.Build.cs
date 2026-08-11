// Copyright RemRemRemRe. 2026. All Rights Reserved.

using UnrealBuildTool;
using Rem.BuildRule;

public class RemAbilityUtilityTest : ModuleRules
{
	public RemAbilityUtilityTest(ReadOnlyTargetRules target) : base(target)
	{
		RemSharedModuleRules.Apply(this);

		PrivateDependencyModuleNames.AddRange(
			[
				"Core",
				"CoreUObject",
				"Engine",

				"GameplayTags",
				"GameplayAbilities",

				"RemCommon",
				"RemCommonTest",
				"RemAbilityUtility",
			]
		);
	}
}
