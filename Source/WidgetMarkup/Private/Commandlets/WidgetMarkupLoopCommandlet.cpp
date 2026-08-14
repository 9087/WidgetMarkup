// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Commandlets/WidgetMarkupLoopCommandlet.h"

#include "Containers/Ticker.h"
#include "DirectoryWatcherModule.h"
#include "IDirectoryWatcher.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformProcess.h"
#include "HAL/ThreadManager.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "StandaloneRenderer.h"
#include "UObject/GarbageCollection.h"
#include "UObject/StrongObjectPtr.h"
#include "WidgetMarkupModule.h"
#include "Widgets/WidgetMarkupWindow.h"

namespace
{
	constexpr int32 ExitSuccess = 0;
	constexpr int32 ExitMissingPackageArg = 2;
	constexpr int32 ExitInvalidPackagePath = 4;
	constexpr int32 ExitModuleLoadFailed = 5;
	constexpr int32 ExitShowWindowFailed = 6;
	constexpr int32 ExitCompileFailed = 7;
	constexpr int32 ExitTestTimeout = 8;
}

UWidgetMarkupLoopCommandlet::UWidgetMarkupLoopCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = false;
	FastExit = true;
	UseCommandletResultAsExitCode = true;
}

int32 UWidgetMarkupLoopCommandlet::Main(const FString& Params)
{
	PRIVATE_GAllowCommandletRendering = true;

	FString PackagePath;
	if (!FParse::Value(*Params, TEXT("WidgetMarkupPackage="), PackagePath) || PackagePath.IsEmpty())
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("Missing -WidgetMarkupPackage=/Root/Path/Asset argument."));
		return ExitMissingPackageArg;
	}

	FText PackagePathError;
	if (!FPackageName::IsValidTextForLongPackageName(PackagePath, &PackagePathError))
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("Invalid package path '%s': %s"), *PackagePath, *PackagePathError.ToString());
		return ExitInvalidPackagePath;
	}

	if (!FSlateApplication::IsInitialized())
	{
		UE_LOG(LogWidgetMarkup, Display, TEXT("Initializing Slate as standalone application."));
		FSlateApplication::InitializeAsStandaloneApplication(GetStandardStandaloneRenderer());

		const bool bWasRunningCommandlet = PRIVATE_GIsRunningCommandlet;
		PRIVATE_GIsRunningCommandlet = false;
		FSlateApplication::InitHighDPI(true);
		PRIVATE_GIsRunningCommandlet = bWasRunningCommandlet;

		FSlateApplication::Get().SetExitRequestedHandler(FSimpleDelegate());
	}

	if (!FModuleManager::Get().LoadModule(TEXT("WidgetMarkup")))
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("Failed to load WidgetMarkup module."));
		return ExitModuleLoadFailed;
	}

	FModuleManager::Get().LoadModule(TEXT("DirectoryWatcher"));
	FWidgetMarkupModule::Get().EnsureSourceFileWatching();

	int32 ExitCode = ExitSuccess;

	TStrongObjectPtr<UWidgetMarkupWindow> WidgetMarkupWindow;
	FWidgetMarkupModule& WidgetMarkupModule = FModuleManager::GetModuleChecked<FWidgetMarkupModule>("WidgetMarkup");

	FString ExtraArguments;
	FParse::Value(*Params, TEXT("ExtraArguments="), ExtraArguments);
	WidgetMarkupModule.SetExtraArguments(ExtraArguments);

	const bool bTestMode = ExtraArguments.Equals(TEXT("test"));
	double TestTimeoutSeconds = 0.0;
	FParse::Value(*Params, TEXT("WidgetMarkupTestTimeout="), TestTimeoutSeconds);

	// The standalone loop never runs the engine's automatic GC, so collect
	// garbage on a fixed interval. Defaults to 60 seconds; <= 0 disables it.
	double GCIntervalSeconds = 60.0;
	FParse::Value(*Params, TEXT("WidgetMarkupGCInterval="), GCIntervalSeconds);

	// In test mode, compile up front and fail fast so test runners can assert
	// the exit code instead of scraping logs for compile errors.
	if (bTestMode && !WidgetMarkupModule.CompileFromPackagePath(PackagePath))
	{
		return ExitCompileFailed;
	}

	FModuleManager::Get().LoadModule(TEXT("PythonScriptPlugin"));
	FModuleManager::Get().LoadModule(TEXT("WidgetMarkupPythonScripting"));

	WidgetMarkupModule.ExecuteOrRegisterOnInitialized(FSimpleDelegate::CreateLambda([PackagePath, &WidgetMarkupWindow, &ExitCode]()
	{
		if (!UWidgetMarkupWindow::CreateAndOpenWidgetMarkupWindow(GetTransientPackage(), PackagePath, WidgetMarkupWindow))
		{
			ExitCode = ExitShowWindowFailed;
		}
	}));

	double LastTime = FPlatformTime::Seconds();
	const double StartTime = LastTime;
	double LastGCTime = LastTime;
	while (!IsEngineExitRequested())
	{
		if (ExitCode != ExitSuccess)
		{
			break;
		}

		if (TestTimeoutSeconds > 0.0 && FPlatformTime::Seconds() - StartTime > TestTimeoutSeconds)
		{
			UE_LOG(LogWidgetMarkup, Error, TEXT("WidgetMarkup test timed out after %.1f seconds."), TestTimeoutSeconds);
			ExitCode = ExitTestTimeout;
			break;
		}

		const double CurrentTime = FPlatformTime::Seconds();
		const float DeltaSeconds = static_cast<float>(CurrentTime - LastTime);
		LastTime = CurrentTime;

		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
		FTSTicker::GetCoreTicker().Tick(DeltaSeconds);

		// Tick DirectoryWatcher so source-file hot-reload works in standalone Program targets.
		if (FModuleManager::Get().IsModuleLoaded("DirectoryWatcher"))
		{
			if (auto* DirectoryWatcherModule = FModuleManager::GetModulePtr<FDirectoryWatcherModule>(FName("DirectoryWatcher")))
			{
				DirectoryWatcherModule->Get()->Tick(DeltaSeconds);
			}
		}

		FSlateApplication::Get().PumpMessages();
		FSlateApplication::Get().Tick();

		FThreadManager::Get().Tick();

		GFrameCounter++;
		FStats::AdvanceFrame(false);

		if (GCIntervalSeconds > 0.0 && CurrentTime - LastGCTime >= GCIntervalSeconds)
		{
			LastGCTime = CurrentTime;
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		}

		if (FSlateApplication::Get().GetInteractiveTopLevelWindows().Num() == 0)
		{
			break;
		}

		FPlatformProcess::Sleep(0.001f);
	}

	// Let the script integration report its own exit code (e.g. Python test
	// failures) so standalone runs and test runners can assert on it.
	if (ExitCode == ExitSuccess)
	{
		ExitCode = WidgetMarkupModule.GetExitCode();
	}

	UE_LOG(LogWidgetMarkup, Display, TEXT("WidgetMarkupApp window closed."));
	return ExitCode;
}
