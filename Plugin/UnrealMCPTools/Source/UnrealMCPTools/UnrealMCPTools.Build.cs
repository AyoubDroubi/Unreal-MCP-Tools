using UnrealBuildTool;

public class UnrealMCPTools : ModuleRules
{
    public UnrealMCPTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "ToolsetRegistry"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "AssetRegistry",
            "BlueprintGraph",
            "DeveloperSettings",
            "MessageLog",
            "Projects",
            "UnrealEd"
        });
    }
}
