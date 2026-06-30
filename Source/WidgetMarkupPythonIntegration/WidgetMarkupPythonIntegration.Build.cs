// Copyright 2025 Wu Zhiwei. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class WidgetMarkupPythonIntegration : ModuleRules
{
	public WidgetMarkupPythonIntegration(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.Never;
		
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
			"Python3",
			"PythonScriptPlugin",
			"Projects",
			"WidgetMarkup",
		});
	}
}
