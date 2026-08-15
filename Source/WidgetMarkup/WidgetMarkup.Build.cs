// Copyright 2025 Wu Zhiwei. All Rights Reserved.

using UnrealBuildTool;

public class WidgetMarkup : ModuleRules
{
	public WidgetMarkup(ReadOnlyTargetRules Target) : base(Target)
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
			"InputCore",
			"SlateCore",
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"ApplicationCore",
			"StandaloneRenderer",
			"Slate",
			"UMG",
			"UMGEditor",
			"BlueprintGraph",
			"UnrealEd",
			"PropertyEditor",
			"XmlParser",
			"DirectoryWatcher",
			"DeveloperSettings",
			"Json",
			"Projects",
		});
	}
}
