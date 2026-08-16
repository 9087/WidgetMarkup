// Copyright 2025 Wu Zhiwei. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class WidgetMarkupPythonScripting : ModuleRules
{
	public WidgetMarkupPythonScripting(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Keep debug builds debuggable, but let Development/Shipping builds
		// use UBT's default optimization settings.
		if (Target.Configuration == UnrealTargetConfiguration.Debug ||
			Target.Configuration == UnrealTargetConfiguration.DebugGame)
		{
			OptimizeCode = CodeOptimization.Never;
		}
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"InputCore",
			"SlateCore",
			"UMG",
			"UMGEditor",
			"Python3",
			"PythonScriptPlugin",
			"Projects",
			"WidgetMarkup",
		});
	}
}
