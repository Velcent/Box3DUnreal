// Author: Antonio Lattanzio - emptyvessel

#include "Box3DUnreal.h"
#include "Box3DLog.h"
#include "Box3DSettings.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogBox3D);

#define LOCTEXT_NAMESPACE "FBox3DUnrealModule"

void FBox3DUnrealModule::StartupModule()
{
	const UBox3DSettings* Settings = GetDefault<UBox3DSettings>();
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("box3d.Enabled")))
	{
		CVar->Set(Settings->bSimulationEnabled ? 1 : 0, ECVF_SetByProjectSetting);
	}
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("box3d.AsyncStep")))
	{
		CVar->Set(Settings->bAsyncStep ? 1 : 0, ECVF_SetByProjectSetting);
	}
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("box3d.LengthUnits")))
	{
		const float Units = Settings->LengthUnits == EBox3DLengthUnits::Centimeters ? 100.0f : 1.0f;
		CVar->Set(Units, ECVF_SetByProjectSetting);
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("DisableBox3D")))
	{
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("box3d.Enabled")))
		{
			CVar->Set(TEXT("0"), ECVF_SetByCommandline);
		}
		UE_LOG(LogBox3D, Log, TEXT("box3d: -DisableBox3D on command line; simulation off (box3d.Enabled=0)."));
	}
}

void FBox3DUnrealModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBox3DUnrealModule, Box3DUnreal)
