using UnrealBuildTool;

public class EggAsync : ModuleRules
{
    public EggAsync(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "EnhancedInput",
            }
            );
    }
}
