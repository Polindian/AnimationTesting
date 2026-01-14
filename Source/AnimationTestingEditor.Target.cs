// Christopher Naglik All Rights Reserved

using UnrealBuildTool;
using System.Collections.Generic;

public class AnimationTestingEditorTarget : TargetRules
{
	public AnimationTestingEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "AnimationTesting" } );
	}
}
