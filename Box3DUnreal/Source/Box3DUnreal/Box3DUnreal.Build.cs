using UnrealBuildTool;

public class Box3DUnreal : ModuleRules
{
	public Box3DUnreal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Box3DLibrary is public because the runtime's public API exposes native Box3D types.
		PublicDependencyModuleNames.AddRange(new[] { "Core", "DeveloperSettings", "Box3DLibrary" });

		// Landscape collision is extracted through ULandscapeHeightfieldCollisionComponent.
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CoreUObject",
			"Engine",
			"PhysicsCore",
			"InputCore",
			"Landscape",
		});
	}
}
