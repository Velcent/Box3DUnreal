// Author: Antonio Lattanzio - emptyvessel

#pragma once

#include "CoreMinimal.h"
#include <box3d/box3d.h>

class AActor;
struct FBox3DBakedBody;

namespace Box3D::StaticGeometry
{
	// Which cooked collision to mirror for a Static body.
	enum class ESource : uint8
	{
		Auto,             // Complex tri-mesh if present, else simple.
		SimpleCollision,  // AggGeom convex/box/sphere/capsule.
		ComplexCollision, // Cooked tri-mesh (meshes & landscape).
	};

	BOX3DUNREAL_API bool ExtractStaticCollision(
		AActor* Owner,
		ESource Source,
		bool bInvertWinding,
		FBox3DBakedBody& Out);

	BOX3DUNREAL_API bool AddBakedShapes(
		b3BodyId Body,
		const b3ShapeDef& Base,
		const FBox3DBakedBody& Baked,
		TArray<b3MeshData*>& OutOwnedMeshes);

	BOX3DUNREAL_API bool AddStaticShapes(
		b3BodyId Body,
		const b3ShapeDef& Base,
		AActor* Owner,
		ESource Source,
		bool bInvertWinding,
		TArray<b3MeshData*>& OutOwnedMeshes);

	/**
	 * Landscape terrain as native box3d height fields, one shape per collision component.
	 *
	 * Separate from ExtractStaticCollision because a landscape has no cooked tri-mesh and no
	 * AggGeom to bake: its collision is a Chaos height field, reachable only through the
	 * Landscape module. The result is also not a FBox3DBakedShape - b3HeightFieldData is an
	 * opaque allocation the shape holds a reference to, so it is created straight onto the body
	 * and handed back for the caller to free with the body.
	 *
	 * Returns the number of height-field shapes created; 0 for an actor that is not a landscape.
	 */
	BOX3DUNREAL_API int32 AddLandscapeHeightFields(
		b3BodyId Body,
		const b3ShapeDef& Base,
		AActor* Owner,
		TArray<b3HeightFieldData*>& OutOwnedHeightFields);

	BOX3DUNREAL_API FString GetBox3DVersionString();
}
