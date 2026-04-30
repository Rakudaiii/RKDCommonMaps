// Copyright (c) 2026 Rakudai. All Rights Reserved.

using UnrealBuildTool;

public class RKDCommonMaps : ModuleRules
{
	public RKDCommonMaps(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",        
			"ToolMenus",      
			"DeveloperSettings", 
			"ContentBrowser"
		});
	}
}
