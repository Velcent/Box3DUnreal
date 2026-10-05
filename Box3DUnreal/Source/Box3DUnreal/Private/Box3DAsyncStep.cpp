// Author: Antonio Lattanzio - emptyvessel


#include "Box3DBodyComponent.h"
#include "Box3DCharacterComponent.h"
#include "Box3DConversion.h"
#include "Box3DLog.h"
#include "Box3DStats.h"
#include "Box3DSubsystem.h"
#include "Box3DAsyncState.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Tasks/Task.h"

namespace
{
	TAutoConsoleVariable<int32> CVarBox3DAsyncStep(
		TEXT("box3d.AsyncStep"),
		1,
		TEXT("Run b3World_Step on a task thread instead of the game thread (1 = on)."),
		ECVF_Default);
}

bool UBox3DSubsystem::IsAsyncStepEnabled()
{
	return CVarBox3DAsyncStep.GetValueOnGameThread() != 0;
}

void UBox3DSubsystem::FlushAsyncStep() const
{
	if (AsyncState != nullptr && AsyncState->StepTask.IsValid())
	{
		// Wait runs it here if it never got scheduled, else blocks.
		AsyncState->StepTask.Wait();
		AsyncState->StepTask = {};
	}
}

bool UBox3DSubsystem::SynchronizeSimulation() const
{
	if (!bWorldValid)
	{
		return false;
	}

	FlushAsyncStep();
	return bWorldValid;
}

void UBox3DSubsystem::GatherKinematicTargets(TArray<FKinematicTarget>& OutTargets)
{
	OutTargets.Reset(KinematicBodies.Num());

	for (int32 Index = KinematicBodies.Num() - 1; Index >= 0; --Index)
	{
		UBox3DBodyComponent* Body = KinematicBodies[Index].Get();
		if (Body == nullptr)
		{
			KinematicBodies.RemoveAtSwap(Index);
			continue;
		}

		const b3BodyId BodyId = Body->GetUnsafeNativeBodyId();
		const AActor* Owner = Body->GetOwner();
		if (B3_IS_NULL(BodyId) || Owner == nullptr)
		{
			continue;
		}

		const FTransform T = Owner->GetActorTransform();

		FKinematicTarget Target;
		Target.Body = BodyId;
		Target.Target.p = Box3D::ToBox3DPosition(T.GetLocation());
		Target.Target.q = Box3D::ToBox3DQuat(T.GetRotation());
		OutTargets.Add(Target);
	}
}

void UBox3DSubsystem::ApplyKinematicTargets(const TArray<FKinematicTarget>& Targets, float TimeStep) const
{
	// Target transform, not teleport: kinematic bodies must carry momentum into contacts.
	for (const FKinematicTarget& Target : Targets)
	{
		b3Body_SetTargetTransform(Target.Body, Target.Target, TimeStep, /*wake=*/true);
	}
}

void UBox3DSubsystem::StepWorldOnly(FBox3DFrameProfile& Frame)
{
	b3World_Step(WorldId, FixedTimeStep, SubStepCount);
	Frame.Accumulate(b3World_GetProfile(WorldId));
}

void FBox3DKickTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType,
	ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionEvent)
{
	if (Subsystem != nullptr)
	{
		Subsystem->KickAsyncStep(DeltaTime);
	}
}

FString FBox3DKickTickFunction::DiagnosticMessage()
{
	return TEXT("FBox3DKickTickFunction");
}

void FBox3DJoinTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType,
	ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionEvent)
{
	if (Subsystem != nullptr)
	{
		Subsystem->JoinAsyncStep();
	}
}

FString FBox3DJoinTickFunction::DiagnosticMessage()
{
	return TEXT("FBox3DJoinTickFunction");
}

void UBox3DSubsystem::RegisterStepTickFunctions()
{
	if (AsyncState == nullptr)
	{
		AsyncState = new FBox3DAsyncState();
	}

	UWorld* World = GetWorld();
	if (AsyncState->bTickFunctionsRegistered || World == nullptr || World->PersistentLevel == nullptr)
	{
		return;
	}

	AsyncState->KickTick.Subsystem = this;
	AsyncState->KickTick.bCanEverTick = true;
	AsyncState->KickTick.bStartWithTickEnabled = true;
	AsyncState->KickTick.TickGroup = TG_PrePhysics;
	AsyncState->KickTick.EndTickGroup = TG_PrePhysics;
	AsyncState->KickTick.bHighPriority = true;
	AsyncState->KickTick.RegisterTickFunction(World->PersistentLevel);

	AsyncState->JoinTick.Subsystem = this;
	AsyncState->JoinTick.bCanEverTick = true;
	AsyncState->JoinTick.bStartWithTickEnabled = true;
	AsyncState->JoinTick.TickGroup = TG_PostPhysics;
	AsyncState->JoinTick.EndTickGroup = TG_PostPhysics;
	AsyncState->JoinTick.RegisterTickFunction(World->PersistentLevel);

	AsyncState->JoinTick.AddPrerequisite(this, AsyncState->KickTick);

	AsyncState->bTickFunctionsRegistered = true;
}

void UBox3DSubsystem::UnregisterStepTickFunctions()
{
	if (AsyncState == nullptr || !AsyncState->bTickFunctionsRegistered)
	{
		return;
	}

	FlushAsyncStep();

	AsyncState->JoinTick.RemovePrerequisite(this, AsyncState->KickTick);
	AsyncState->KickTick.UnRegisterTickFunction();
	AsyncState->JoinTick.UnRegisterTickFunction();
	AsyncState->KickTick.Subsystem = nullptr;
	AsyncState->JoinTick.Subsystem = nullptr;
	AsyncState->bTickFunctionsRegistered = false;
}

void UBox3DSubsystem::KickAsyncStep(float DeltaTime)
{
	if (!bWorldValid || !IsAsyncStepEnabled())
	{
		return; // synchronous mode steps from Tick instead
	}

	Accumulator = FMath::Min(Accumulator + DeltaTime, static_cast<double>(MaxFrameTime));

	if (Accumulator < FixedTimeStep)
	{
		AsyncState->StepCount = 0;
		return; // not enough time banked for a step this frame
	}

	AsyncState->StepCount = 1;

	TArray<FKinematicTarget> Targets;
	GatherKinematicTargets(Targets);

	AsyncFrame = FBox3DFrameProfile();

	AsyncState->StepTask = UE::Tasks::Launch(UE_SOURCE_LOCATION,
		[this, Targets = MoveTemp(Targets)]()
		{
			ApplyKinematicTargets(Targets, FixedTimeStep);
			StepWorldOnly(AsyncFrame);
		});
}

void UBox3DSubsystem::JoinAsyncStep()
{
	if (!bWorldValid)
	{
		return;
	}

	FlushAsyncStep();

	if (AsyncState == nullptr || AsyncState->StepCount == 0)
	{
		return;
	}

	// Catch-up steps run inline, one at a time, so each drain sees its own events.
	AsyncState->StepCount = 0;
	FinishStepGameThread();

	while (Accumulator >= FixedTimeStep)
	{
		TArray<FKinematicTarget> Targets;
		GatherKinematicTargets(Targets);
		ApplyKinematicTargets(Targets, FixedTimeStep);

		StepWorldOnly(AsyncFrame);
		FinishStepGameThread();
	}

	PublishStats(AsyncFrame);
	ApplyRenderInterpolation();

	DispatchPendingEvents();
}
