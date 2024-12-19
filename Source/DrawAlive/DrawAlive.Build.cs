using UnrealBuildTool;

public class DrawAlive : ModuleRules
{
	public DrawAlive(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"DesktopPlatform", // Dodane, aby korzystać z okna dialogowego systemu operacyjnego
			"EnhancedInput", // Dodane do obsługi rozbudowanego systemu wejść
			"Slate", // Dodane do korzystania z FSlateApplication
			"SlateCore", // Dodane do korzystania z FSlateApplication
			"UMG",        // Dodane
			"Slate",      // Dodane
			"SlateCore" 
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}