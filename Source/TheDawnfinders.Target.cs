// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class TheDawnfindersTarget : TargetRules
{
	public TheDawnfindersTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		
		if (Configuration == UnrealTargetConfiguration.Shipping)
		{
			GlobalDefinitions.Add("ALLOW_CONSOLE_IN_SHIPPING=1");
			bUseLoggingInShipping = true; 
		}

		ExtraModuleNames.AddRange( new string[] { "TheDawnfinders" } );
	}
}
