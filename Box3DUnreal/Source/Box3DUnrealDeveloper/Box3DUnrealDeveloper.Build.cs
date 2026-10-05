using UnrealBuildTool;

public class Box3DUnrealDeveloper : ModuleRules
{
	public Box3DUnrealDeveloper(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Box3DUnreal",
		});
	}
}
