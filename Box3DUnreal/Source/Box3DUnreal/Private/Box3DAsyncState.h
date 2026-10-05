#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Tasks/Task.h"
#include "Box3DAsyncState.generated.h"

class UBox3DSubsystem;

USTRUCT()
struct FBox3DKickTickFunction : public FTickFunction
{
	GENERATED_BODY()

	UBox3DSubsystem* Subsystem = nullptr;

	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType,
		ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionEvent) override;
	virtual FString DiagnosticMessage() override;
};

template<>
struct TStructOpsTypeTraits<FBox3DKickTickFunction> : public TStructOpsTypeTraitsBase2<FBox3DKickTickFunction>
{
	enum { WithCopy = false };
};

USTRUCT()
struct FBox3DJoinTickFunction : public FTickFunction
{
	GENERATED_BODY()

	UBox3DSubsystem* Subsystem = nullptr;

	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType,
		ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionEvent) override;
	virtual FString DiagnosticMessage() override;
};

template<>
struct TStructOpsTypeTraits<FBox3DJoinTickFunction> : public TStructOpsTypeTraitsBase2<FBox3DJoinTickFunction>
{
	enum { WithCopy = false };
};

struct FBox3DAsyncState
{
	mutable UE::Tasks::FTask StepTask;
	FBox3DKickTickFunction KickTick;
	FBox3DJoinTickFunction JoinTick;
	int32 StepCount = 0;
	bool bTickFunctionsRegistered = false;
};
