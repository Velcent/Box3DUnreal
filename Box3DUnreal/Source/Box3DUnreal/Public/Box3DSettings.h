// Author: Antonio Lattanzio - emptyvessel

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Box3DSettings.generated.h"

UENUM()
enum class EBox3DLengthUnits : uint8
{
	Meters,
	Centimeters,
};

/** Project-wide Box3D defaults. Console variables remain available as runtime overrides. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Box3D"))
class BOX3DUNREAL_API UBox3DSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	UPROPERTY(Config, EditAnywhere, Category = "Simulation")
	bool bSimulationEnabled = true;

	UPROPERTY(Config, EditAnywhere, Category = "Simulation")
	bool bAsyncStep = true;

	/** Native Box3D units per meter. Changing this requires recreating the Box3D world. */
	UPROPERTY(Config, EditAnywhere, Category = "Simulation")
	EBox3DLengthUnits LengthUnits = EBox3DLengthUnits::Meters;

	UPROPERTY(Config, EditAnywhere, Category = "Simulation", meta = (ClampMin = "0.001", UIMin = "0.008333", UIMax = "0.033333"))
	float FixedTimeStep = 1.0f / 60.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Simulation", meta = (ClampMin = "1", UIMin = "1", UIMax = "16"))
	int32 SolverSubSteps = 4;

	/** Maximum real time consumed in one frame, preventing an unbounded catch-up loop. */
	UPROPERTY(Config, EditAnywhere, Category = "Simulation", meta = (ClampMin = "0.001"))
	float MaxFrameTime = 0.25f;

	UPROPERTY(Config, EditAnywhere, Category = "World")
	FVector Gravity = FVector(0.0, 0.0, -980.0);

	/** Keep at one when deterministic results are required. */
	UPROPERTY(Config, EditAnywhere, Category = "World", meta = (ClampMin = "1"))
	int32 WorkerCount = 1;

	/** Minimum impact speed in cm/s for hit events. */
	UPROPERTY(Config, EditAnywhere, Category = "Events", meta = (ClampMin = "0.0"))
	float HitEventThreshold = 100.0f;
};
