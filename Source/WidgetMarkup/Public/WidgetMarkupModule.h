// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "ElementNode.h"
#include "PropertyBuffer.h"
#include "PropertyRun.h"
#include "PropertySetter.h"
#include "Utilities/WidgetPropertyPath.h"
#include "WidgetMarkupScriptIntegration.h"

WIDGETMARKUP_API DECLARE_LOG_CATEGORY_EXTERN(LogWidgetMarkup, Log, All);

class FWidgetMarkupModule;
class UUserWidget;
class UWidgetMarkupBlueprintGeneratedClassExtension;

class WIDGETMARKUP_API FWidgetMarkupModule : public IModuleInterface, public FGCObject
{
public:
	static constexpr const TCHAR* SourceFileExtension = TEXT(".widgetmarkup");

	/** Delay used to coalesce bursts of directory-watcher events into a single recompile batch. */
	static constexpr float CompileDebounceDelaySeconds = 0.15f;

	static FWidgetMarkupModule& Get();

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	UObject* CompileFromPackagePath(const FString& PackagePath);
	UObject* GetObjectFromPackagePath(const FString& PackagePath);
	UObject* GetObjectOrCompileFromPackage(const FString& PackagePath);

	/** Access the compiled in-memory objects keyed by long package path (for tooling and script integrations). */
	const TMap<FName, TObjectPtr<UObject>>& GetCompiledObjects() const { return Objects; }

	/** Access the active script integration (may be null). */
	TSharedPtr<IWidgetMarkupScriptIntegration> GetScriptIntegration() const { return ScriptIntegration; }

	/** Returns the last compile error for a package path, or null if the last compile succeeded. */
	const FText* GetLastCompileError(FName PackagePath) const { return LastCompileErrors.Find(PackagePath); }

	template <typename T>
	T* CompileFromPackagePath(const FString& PackagePath)
	{
		return Cast<T>(CompileFromPackagePath(PackagePath));
	}

	template <typename T>
	T* GetObjectFromPackagePath(const FString& PackagePath)
	{
		return Cast<T>(GetObjectFromPackagePath(PackagePath));
	}

	template <typename T>
	T* GetObjectOrCompileFromPackage(const FString& PackagePath)
	{
		return Cast<T>(GetObjectOrCompileFromPackage(PackagePath));
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnObjectCompiled, FName, UObject*)
	FOnObjectCompiled& GetOnObjectCompiled();

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWidgetMarkupUserWidgetInitialized, UUserWidget*, UWidgetMarkupBlueprintGeneratedClassExtension*)
	FOnWidgetMarkupUserWidgetInitialized& GetOnWidgetMarkupUserWidgetInitialized();

	DECLARE_DELEGATE_RetVal(TSharedRef<IPropertyRun>, FOnCreatePropertyRun);
	bool RegisterCustomPropertyRun(UStruct* InStruct, FName InPropertyPath, FOnCreatePropertyRun InOnCreatePropertyRun);
	void UnregisterCustomPropertyRun(UStruct* InStruct, FName InPropertyPath);

	template <typename T>
	bool RegisterCustomPropertyRun(FName InPropertyPath, FOnCreatePropertyRun InOnCreatePropertyRun)
	{
		if constexpr (TIsDerivedFrom<T, UObject>::Value)
		{
			return RegisterCustomPropertyRun(T::StaticClass(), InPropertyPath, InOnCreatePropertyRun);
		}
		else
		{
			return RegisterCustomPropertyRun(T::StaticStruct(), InPropertyPath, InOnCreatePropertyRun);
		}
	}

	template <typename T>
	void UnregisterCustomPropertyRun(FName InPropertyPath)
	{
		if constexpr (TIsDerivedFrom<T, UObject>::Value)
		{
			UnregisterCustomPropertyRun(T::StaticClass(), InPropertyPath);
		}
		else
		{
			UnregisterCustomPropertyRun(T::StaticStruct(), InPropertyPath);
		}
	}

	TSharedPtr<IPropertyRun> CreateCustomPropertyRun(UStruct* InStruct, FName InPropertyPath) const;
	TSharedRef<IPropertyRun> CreatePropertyRun(UStruct* InStruct, FName InPropertyPath) const;

	/** Returns the canonical names of custom property runs that apply to the given struct (e.g. "Script", "ListItems"). */
	TArray<FName> GetCustomPropertyRunNames(UStruct* InStruct) const;

	DECLARE_DELEGATE_RetVal(TSharedRef<FPropertySetter>, FOnCreatePropertySetter);
	bool RegisterCustomPropertySetter(UStruct* InStruct, FName InPropertyPath, FOnCreatePropertySetter InOnCreatePropertySetter);
	void UnregisterCustomPropertySetter(UStruct* InStruct, FName InPropertyPath);

	template <typename T>
	bool RegisterCustomPropertySetter(FName InPropertyPath, FOnCreatePropertySetter InOnCreatePropertySetter)
	{
		if constexpr (TIsDerivedFrom<T, UObject>::Value)
		{
			return RegisterCustomPropertySetter(T::StaticClass(), InPropertyPath, InOnCreatePropertySetter);
		}
		else
		{
			return RegisterCustomPropertySetter(T::StaticStruct(), InPropertyPath, InOnCreatePropertySetter);
		}
	}

	template <typename T>
	void UnregisterCustomPropertySetter(FName InPropertyPath)
	{
		if constexpr (TIsDerivedFrom<T, UObject>::Value)
		{
			UnregisterCustomPropertySetter(T::StaticClass(), InPropertyPath);
		}
		else
		{
			UnregisterCustomPropertySetter(T::StaticStruct(), InPropertyPath);
		}
	}

	TSharedPtr<FPropertySetter> CreateCustomPropertySetter(UStruct* InStruct, FName InPropertyPath) const;
	bool ApplyPropertyValue(UObject* TargetObject, const FWidgetPropertyPath& PropertyPath, const FStringView& ValueString, FText* OutError = nullptr) const;
	bool ApplyPropertyValue(UObject* TargetObject, const FWidgetPropertyPath& PropertyPath, const FPropertyBuffer& PropertyBuffer, FText* OutError = nullptr) const;

private:
	UObject* CompileFromSourceCode(FName PackagePath, const FString& XML);

	TMap<FName, TObjectPtr<UObject>> Objects;
	/** Last compile error text per normalized package path; removed on success. */
	TMap<FName, FText> LastCompileErrors;
	FOnObjectCompiled OnObjectCompiled;
	FOnWidgetMarkupUserWidgetInitialized OnWidgetMarkupUserWidgetInitialized;

	/** Custom properties: keyed by UStruct* (element type), then exact canonical property path to descriptor. */
	TMap<TWeakObjectPtr<UStruct>, TMap<FWidgetPropertyPath, FOnCreatePropertyRun>> PropertyRunCreateDelegates;
	/** Custom property setters: keyed by UStruct* (element type), then exact canonical property path to setter factory. */
	TMap<TWeakObjectPtr<UStruct>, TMap<FWidgetPropertyPath, FOnCreatePropertySetter>> PropertySetterCreateDelegates;

public:
	void OnPostEngineInit();
	void EnsureSourceFileWatching();
	void StartSourceFileWatching(const FDirectoryPath& InDirectoryPath);
	void StopSourceFileWatching();

private:
	void EnsureRemoteControlPreset();
	void HandleOnSourceFileDirectoryChanged(const TArray<struct FFileChangeData>& FileChanges, const FString& WatchedDirectory);
	void EnsureCompileDebounceTicker();
	bool TickCompileDebounce(float DeltaSeconds);

	TMap<FString, FName> SourceFileToName;
	/** Maps absolute watched directory path -> delegate handle. Supports multiple watched directories. */
	TMap<FString, FDelegateHandle> WatchedDirectories;

	/** Package paths waiting to be recompiled after the debounce window. */
	TSet<FName> PendingCompilePaths;
	FTSTicker::FDelegateHandle CompileDebounceTickerHandle;

	/** Remote Control preset exposing the attribute discovery library. */
	TObjectPtr<class URemoteControlPreset> AttributePreset;

public:
	DECLARE_MULTICAST_DELEGATE(FOnInitialized);
	FOnInitialized& GetOnInitialized() { return OnInitialized; }
	void ExecuteOrRegisterOnInitialized(FSimpleDelegate InCallback);
	bool IsInitialized() const { return bInitialized; }
	void NotifyInitialized();

	template <typename T>
	void StartUp()
	{
		StartUp(MakeShared<T>(*this));
	}

	void StartUp(TSharedRef<IWidgetMarkupScriptIntegration> InScriptIntegration);
	void Shutdown();

	const FString& GetExtraArguments() const { return ExtraArguments; }
	void SetExtraArguments(const FString& InExtraArguments) { ExtraArguments = InExtraArguments; }

	/** Process exit code reported by the script integration (e.g. test runs). */
	int32 GetExitCode() const { return ExitCode; }
	void SetExitCode(int32 InExitCode) { ExitCode = InExitCode; }

private:
	FOnInitialized OnInitialized;
	bool bInitialized = false;
	TArray<FSimpleDelegate> PendingInitializedCallbacks;
	TSharedPtr<IWidgetMarkupScriptIntegration> ScriptIntegration;
	FString ExtraArguments;
	int32 ExitCode = 0;
};
