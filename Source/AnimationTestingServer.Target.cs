// Christopher Naglik All Rights Reserved

using UnrealBuildTool;
using System.Collections.Generic;

public class AnimationTestingServerTarget : TargetRules
{
	public AnimationTestingServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "AnimationTesting" } );
	}
}
