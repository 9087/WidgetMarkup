// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "WidgetMarkupModule.h"

#include "ConverterRegistry.h"
#include "DirectoryWatcherModule.h"
#include "Editor.h"
#include "Editor/TransBuffer.h"
#include "ElementNodeFactory.h"
#include "FastXml.h"
#include "ElementTreeBuilder.h"
#include "IDirectoryWatcher.h"
#include "IRemoteControlModule.h"
#include "RemoteControlPreset.h"
#include "WidgetMarkupSettings.h"
#include "WidgetMarkupLibrary.h"
#include "Engine/Blueprint.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Components/SlateWrapperTypes.h"
#include "Converters/BooleanConverter.h"
#include "Converters/ClassConverter.h"
#include "Converters/ColorConverter.h"
#include "Converters/EnumConverter.h"
#include "Converters/ObjectConverter.h"
#include "Converters/SoftObjectConverter.h"
#include "Converters/LinearColorConverter.h"
#include "Converters/MarginConverter.h"
#include "Converters/NameConverter.h"
#include "Converters/NumericConverter.h"
#include "Converters/StringConverter.h"
#include "Converters/TextConverter.h"
#include "Converters/VectorConverter.h"
#include "Converters/WidgetPropertyPathConverter.h"
#include "Data/WidgetMarkupKeyValuePair.h"
#include "ElementNodes/BlueprintElementNode.h"
#include "ElementNodes/BlueprintVariableElementNode.h"
#include "ElementNodes/PropertyChainHandle.h"
#include "ElementNodes/WidgetMarkupBlueprintVariable.h"
#include "PropertySetters/ResyncPropertySetter.h"
#include "PropertyRuns/BlueprintImplementsPropertyRun.h"
#include "PropertyRuns/BlueprintSuperPropertyRun.h"
#include "PropertyRuns/ObjectNamePropertyRun.h"
#include "PropertyRuns/StyleSheetInheritPropertyRun.h"
#include "PropertyRuns/WidgetDelegatePropertyRun.h"
#include "PropertyRuns/VariableDefaultPropertyRun.h"
#include "Registrations/WidgetRegistrations.h"
#include "Styles/WidgetStyleSheet.h"
#include "Utilities/WidgetPropertyPath.h"
#include "ElementNodes/SetterElementNode.h"
#include "ElementNodes/StyleElementNode.h"
#include "ElementNodes/StyleSheetElementNode.h"
#include "ElementNodes/StructElementNode.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/PackageName.h"

DEFINE_LOG_CATEGORY(LogWidgetMarkup);

IWidgetMarkupScriptIntegration::IWidgetMarkupScriptIntegration(FWidgetMarkupModule& InWidgetMarkupModule)
	: WidgetMarkupModule(InWidgetMarkupModule)
{
}

void IWidgetMarkupScriptIntegration::Initialize(bool bOK)
{
	if (bOK)
	{
		this->WidgetMarkupModule.NotifyInitialized();
	}
}

FWidgetMarkupModule& FWidgetMarkupModule::Get()
{
	return FModuleManager::GetModuleChecked<FWidgetMarkupModule>("WidgetMarkup");
}

void FWidgetMarkupModule::StartupModule()
{
	// Ensure the plugin's Content directory is mounted so subsystems (e.g. PythonScriptPlugin)
	// can discover assets and scripts via the plugin's mounted asset path.
	if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("WidgetMarkup")))
	{
		const FString MountedAssetPath = Plugin->GetMountedAssetPath();
		const FString ContentDir = Plugin->GetContentDir();
		if (!MountedAssetPath.IsEmpty() && !ContentDir.IsEmpty() && !FPackageName::MountPointExists(MountedAssetPath))
		{
			FPackageName::RegisterMountPoint(MountedAssetPath, ContentDir);
			UE_LOG(LogWidgetMarkup, Display, TEXT("WidgetMarkup mounted content root: %s -> %s"), *MountedAssetPath, *ContentDir);
		}
	}
	else
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("Failed to locate plugin 'WidgetMarkup' while mounting Content directory."));
	}

	FElementNodeFactory::Get().Register<UBlueprint>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FBlueprintElementNode::Create));
	FElementNodeFactory::Get().Register<FWidgetMarkupBlueprintVariable>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FBlueprintVariableElementNode::Create), FElementNodeFactory::FRegisterOptions{FString(TEXT("Variable"))});
	FElementNodeFactory::Get().Register<FWidgetMarkupKeyValuePair>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FStructElementNode::Create), FElementNodeFactory::FRegisterOptions{FString(TEXT("Pair"))});
	FElementNodeFactory::Get().Register<UWidgetStyleSheet>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FStyleSheetElementNode::Create), FElementNodeFactory::FRegisterOptions{FString(TEXT("StyleSheet"))});
	FElementNodeFactory::Get().Register<FWidgetStyleEntry>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FStyleElementNode::Create), FElementNodeFactory::FRegisterOptions{FString(TEXT("Style"))});
	FElementNodeFactory::Get().Register<FWidgetStyleSetter>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FSetterElementNode::Create), FElementNodeFactory::FRegisterOptions{FString(TEXT("Setter"))});

	FConverterRegistry::Get().Register(NAME_ByteProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<uint8>::Create));
	FConverterRegistry::Get().Register(NAME_IntProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<int>::Create));
	FConverterRegistry::Get().Register(NAME_BoolProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(FBooleanConverter::Create));
	FConverterRegistry::Get().Register(NAME_FloatProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<float>::Create));
	FConverterRegistry::Get().Register(NAME_NameProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(FNameConverter::Create));
	FConverterRegistry::Get().Register(NAME_DoubleProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<double>::Create));
	FConverterRegistry::Get().Register(NAME_StrProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(FStringConverter::Create));
	FConverterRegistry::Get().Register(NAME_TextProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(FTextConverter::Create));
	FConverterRegistry::Get().Register(NAME_Int64Property, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<int64>::Create));
	FConverterRegistry::Get().Register(NAME_Int32Property, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<int32>::Create));
	FConverterRegistry::Get().Register(NAME_Int16Property, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<int16>::Create));
	FConverterRegistry::Get().Register(NAME_Int8Property, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<int8>::Create));
	FConverterRegistry::Get().Register(NAME_UInt64Property, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<uint64>::Create));
	FConverterRegistry::Get().Register(NAME_UInt32Property, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<uint32>::Create));
	FConverterRegistry::Get().Register(NAME_UInt16Property, FConverterRegistry::FOnCreateConverter::CreateStatic(TNumericConverter<uint16>::Create));
	FConverterRegistry::Get().Register(NAME_EnumProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(FEnumConverter::Create));
	FConverterRegistry::Get().Register(FClassProperty::StaticClass()->GetFName(), FConverterRegistry::FOnCreateConverter::CreateStatic(FClassConverter::Create));
	FConverterRegistry::Get().Register(FSoftClassProperty::StaticClass()->GetFName(), FConverterRegistry::FOnCreateConverter::CreateStatic(FClassConverter::Create));
	FConverterRegistry::Get().Register(NAME_Color, FConverterRegistry::FOnCreateConverter::CreateStatic(FColorConverter::Create));
	FConverterRegistry::Get().Register(NAME_LinearColor, FConverterRegistry::FOnCreateConverter::CreateStatic(FLinearColorConverter::Create));
	FConverterRegistry::Get().Register(FMargin::StaticStruct()->GetFName(), FConverterRegistry::FOnCreateConverter::CreateStatic(FMarginConverter::Create));
	FConverterRegistry::Get().Register(NAME_Vector, FConverterRegistry::FOnCreateConverter::CreateStatic(TVectorConverter<FVector::FReal, 3>::Create));
	FConverterRegistry::Get().Register(NAME_Vector2D, FConverterRegistry::FOnCreateConverter::CreateStatic(TVectorConverter<FVector2D::FReal, 2>::Create));
	FConverterRegistry::Get().Register(TBaseStructure<FVector4>::Get()->GetFName(), FConverterRegistry::FOnCreateConverter::CreateStatic(TVectorConverter<decltype(FVector4::X), 4>::Create));
	FConverterRegistry::Get().Register(FWidgetPropertyPath::StaticStruct()->GetFName(), FConverterRegistry::FOnCreateConverter::CreateStatic(FWidgetPropertyPathConverter::Create));
	FConverterRegistry::Get().Register(NAME_ObjectProperty, FConverterRegistry::FOnCreateConverter::CreateStatic(FObjectConverter::Create));
	FConverterRegistry::Get().Register(FSoftObjectProperty::StaticClass()->GetFName(), FConverterRegistry::FOnCreateConverter::CreateStatic(FSoftObjectConverter::Create));
	
	RegisterCustomPropertyRun(UObject::StaticClass(), TEXT("Name"), FOnCreatePropertyRun::CreateStatic(&FObjectNamePropertyRun::Create));
	RegisterCustomPropertyRun(UBlueprint::StaticClass(), TEXT("Super"), FOnCreatePropertyRun::CreateStatic(&FBlueprintSuperPropertyRun::Create));
	RegisterCustomPropertyRun(UBlueprint::StaticClass(), TEXT("Implements"), FOnCreatePropertyRun::CreateStatic(&FBlueprintImplementsPropertyRun::Create));
	RegisterCustomPropertyRun(UWidgetStyleSheet::StaticClass(), TEXT("Inherit"), FOnCreatePropertyRun::CreateStatic(&FStyleSheetInheritPropertyRun::Create));
	RegisterCustomPropertyRun(FWidgetMarkupBlueprintVariable::StaticStruct(), TEXT("Default"), FOnCreatePropertyRun::CreateStatic(&FVariableDefaultPropertyRun::Create));

	// UMG/Slate registrations live in Registrations/WidgetRegistrations.cpp.
	RegisterWidgetRegistrations(*this);
	
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FWidgetMarkupModule::OnPostEngineInit);
}

void FWidgetMarkupModule::ShutdownModule()
{
	FElementNodeFactory::Get().Unregister<UBlueprint>();
	FElementNodeFactory::Get().Unregister<FWidgetMarkupBlueprintVariable>();
	FElementNodeFactory::Get().Unregister<FWidgetMarkupKeyValuePair>();
	FElementNodeFactory::Get().Unregister<UWidgetStyleSheet>();
	FElementNodeFactory::Get().Unregister<FWidgetStyleEntry>();
	FElementNodeFactory::Get().Unregister<FWidgetStyleSetter>();

	FConverterRegistry::Get().Unregister(NAME_ByteProperty);
	FConverterRegistry::Get().Unregister(NAME_IntProperty);
	FConverterRegistry::Get().Unregister(NAME_BoolProperty);
	FConverterRegistry::Get().Unregister(NAME_FloatProperty);
	FConverterRegistry::Get().Unregister(NAME_NameProperty);
	FConverterRegistry::Get().Unregister(NAME_DoubleProperty);
	FConverterRegistry::Get().Unregister(NAME_StrProperty);
	FConverterRegistry::Get().Unregister(NAME_TextProperty);
	FConverterRegistry::Get().Unregister(NAME_Int64Property);
	FConverterRegistry::Get().Unregister(NAME_Int32Property);
	FConverterRegistry::Get().Unregister(NAME_Int16Property);
	FConverterRegistry::Get().Unregister(NAME_Int8Property);
	FConverterRegistry::Get().Unregister(NAME_UInt64Property);
	FConverterRegistry::Get().Unregister(NAME_UInt32Property);
	FConverterRegistry::Get().Unregister(NAME_UInt16Property);
	FConverterRegistry::Get().Unregister(NAME_EnumProperty);
	FConverterRegistry::Get().Unregister(FClassProperty::StaticClass()->GetFName());
	FConverterRegistry::Get().Unregister(FSoftClassProperty::StaticClass()->GetFName());
	FConverterRegistry::Get().Unregister(NAME_Color);
	FConverterRegistry::Get().Unregister(NAME_LinearColor);
	FConverterRegistry::Get().Unregister(FMargin::StaticStruct()->GetFName());
	FConverterRegistry::Get().Unregister(NAME_Vector);
	FConverterRegistry::Get().Unregister(NAME_Vector2D);
	FConverterRegistry::Get().Unregister(TBaseStructure<FVector4>::Get()->GetFName());
	FConverterRegistry::Get().Unregister(FWidgetPropertyPath::StaticStruct()->GetFName());
	FConverterRegistry::Get().Unregister(NAME_ObjectProperty);
	FConverterRegistry::Get().Unregister(FSoftObjectProperty::StaticClass()->GetFName());

	UnregisterCustomPropertyRun(UObject::StaticClass(), TEXT("Name"));
	UnregisterCustomPropertyRun(UBlueprint::StaticClass(), TEXT("Super"));
	UnregisterCustomPropertyRun(UBlueprint::StaticClass(), TEXT("Implements"));
	UnregisterCustomPropertyRun(UWidgetStyleSheet::StaticClass(), TEXT("Inherit"));
	UnregisterCustomPropertyRun(FWidgetMarkupBlueprintVariable::StaticStruct(), TEXT("Default"));

	// UMG/Slate registrations live in Registrations/WidgetRegistrations.cpp.
	UnregisterWidgetRegistrations(*this);

	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	StopSourceFileWatching();

	if (AttributePreset && FModuleManager::Get().IsModuleLoaded(TEXT("RemoteControl")))
	{
		IRemoteControlModule& RemoteControlModule = IRemoteControlModule::Get();
		RemoteControlModule.UnregisterEmbeddedPreset(AttributePreset.Get());
		RemoteControlModule.DestroyTransientPreset(AttributePreset->GetPresetId());
	}
	AttributePreset = nullptr;

	if (CompileDebounceTickerHandle.IsValid())
	{
		FTSTicker::RemoveTicker(CompileDebounceTickerHandle);
		CompileDebounceTickerHandle.Reset();
	}
	PendingCompilePaths.Empty();
	PropertyRunCreateDelegates.Empty();
	PropertySetterCreateDelegates.Empty();
}

// ---------------------------------------------------------------------------
// Path helpers: PackagePath (/Game/.../AssetName) <-> absolute disk file path
// ---------------------------------------------------------------------------

// /Game/WidgetMarkup/Foo  ->  <ContentDir>/WidgetMarkup/Foo.widgetmarkup
// /PluginName/Foo         ->  <PluginContentDir>/Foo.widgetmarkup
// Uses FPackageName to support all mounted content roots (project + plugins).
static bool TryConvertPackagePathToAbsoluteSourceFilePath(const FString& PackagePath, FStringView Extension, FString& OutAbsoluteFilePath)
{
	return FPackageName::TryConvertLongPackageNameToFilename(PackagePath, OutAbsoluteFilePath, FString(Extension));
}

// <ContentDir>/WidgetMarkup/Foo.widgetmarkup    ->  /Game/WidgetMarkup/Foo
// <PluginContentDir>/Foo.widgetmarkup           ->  /PluginName/Foo
// Uses FPackageName to support all mounted content roots (project + plugins).
static bool TryConvertAbsoluteSourceFilePathToPackagePath(const FString& AbsoluteFilePath, FStringView Extension, FString& OutPackagePath)
{
	if (FPaths::IsRelative(AbsoluteFilePath))
	{
		return false;
	}

	// Strip custom extension before passing to FPackageName (it only knows .uasset/.umap etc.)
	FString FilePathWithoutExt = AbsoluteFilePath;
	const FString ExtensionString(Extension);
	if (FilePathWithoutExt.EndsWith(ExtensionString))
	{
		FilePathWithoutExt = FilePathWithoutExt.LeftChop(ExtensionString.Len());
	}
	return FPackageName::TryConvertFilenameToLongPackageName(FilePathWithoutExt, OutPackagePath);
}

// FFastXml declares its contents buffer as non-const but never writes to it
// (verified in FastXml.cpp: it only measures the length and walks the buffer),
// so keep the const_cast contained here instead of at every call site.
static bool TryParseWidgetMarkupXml(IFastXmlCallback& Callback, const FString& XML, FText& OutErrorMessage, int32& OutErrorLineNumber)
{
	return FFastXml::ParseXmlFile(&Callback, nullptr, const_cast<TCHAR*>(*XML), GWarn, true, false, OutErrorMessage, OutErrorLineNumber);
}

// ---------------------------------------------------------------------------

TSharedPtr<IPropertyRun> FWidgetMarkupModule::CreateCustomPropertyRun(UStruct* InStruct, FName InPropertyPath) const
{
	const FString PropertyPathString = InPropertyPath.ToString();

	if (!InStruct)
	{
		return nullptr;
	}

	FWidgetPropertyPath PropertyPath;
	FString ParseError;
	if (!FWidgetPropertyPath::TryParse(PropertyPathString, PropertyPath, &ParseError))
	{
		return nullptr;
	}

	UStruct* BestStruct = nullptr;
	const FOnCreatePropertyRun* BestDelegate = nullptr;
	for (const auto& KeyValuePair : PropertyRunCreateDelegates)
	{
		UStruct* Struct = KeyValuePair.Key.Get();
		if (!Struct)
		{
			continue;
		}
		if (!InStruct->IsChildOf(Struct))
		{
			continue;
		}

		const FOnCreatePropertyRun* Found = KeyValuePair.Value.Find(PropertyPath);
		if (!Found || !Found->IsBound())
		{
			continue;
		}

		if (!BestStruct || Struct->IsChildOf(BestStruct))
		{
			BestStruct = Struct;
			BestDelegate = Found;
		}
	}
	if (!BestDelegate)
	{
		return nullptr;
	}
	return BestDelegate->Execute();
}

TArray<FName> FWidgetMarkupModule::GetCustomPropertyRunNames(UStruct* InStruct) const
{
	TArray<FName> Names;
	if (!InStruct)
	{
		return Names;
	}

	for (const auto& KeyValuePair : PropertyRunCreateDelegates)
	{
		UStruct* Struct = KeyValuePair.Key.Get();
		if (!Struct || !InStruct->IsChildOf(Struct))
		{
			continue;
		}

		for (const auto& PathPair : KeyValuePair.Value)
		{
			Names.AddUnique(FName(*PathPair.Key.GetPathName().ToString()));
		}
	}
	return Names;
}

TSharedRef<IPropertyRun> FWidgetMarkupModule::CreatePropertyRun(UStruct* InStruct, FName InPropertyPath) const
{
	TSharedPtr<IPropertyRun> CustomPropertyRun = CreateCustomPropertyRun(InStruct, InPropertyPath);
	if (CustomPropertyRun.IsValid())
	{
		return CustomPropertyRun.ToSharedRef();
	}

	// Auto-detect delegate properties: multicast and single-cast dynamic delegate
	// attributes on a widget are forwarded to FWidgetDelegatePropertyRun.
	if (InStruct && InStruct->IsChildOf<UWidget>())
	{
		if (FProperty* Property = InStruct->FindPropertyByName(InPropertyPath))
		{
			if (Property->IsA<FMulticastDelegateProperty>() || Property->IsA<FDelegateProperty>())
			{
				return FWidgetDelegatePropertyRun::Create();
			}
		}
	}

	return MakeShared<FPropertyRun>();
}

bool FWidgetMarkupModule::RegisterPropertyResync(UStruct* InStruct, FName InPropertyPath, FPropertyResyncDelegate InResync, bool (*InIsCompatible)(const FProperty&), const TCHAR* InExpectedTypeName)
{
	if (!InStruct || !InResync)
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("RegisterPropertyResync: invalid struct or empty callback."));
		return false;
	}

	// Only direct properties are supported: with the exact-path dispatch below this
	// also guarantees that the tail container passed to the setter is the widget
	// object itself, not a nested struct.
	FProperty* Property = InStruct->FindPropertyByName(InPropertyPath);
	if (!Property)
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("RegisterPropertyResync failed: '%s' has no direct property '%s'."),
			*InStruct->GetName(), *InPropertyPath.ToString());
		return false;
	}

	if (InIsCompatible && !InIsCompatible(*Property))
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("RegisterPropertyResync failed: '%s.%s' is a '%s', which cannot hold '%s'."),
			*InStruct->GetName(), *InPropertyPath.ToString(), *Property->GetClass()->GetName(),
			InExpectedTypeName ? InExpectedTypeName : TEXT("<unknown>"));
		return false;
	}

	return RegisterCustomPropertySetter(InStruct, InPropertyPath, FOnCreatePropertySetter::CreateLambda([InResync = MoveTemp(InResync)]()
	{
		return FResyncPropertySetter::Create(InResync);
	}));
}

TSharedPtr<FPropertySetter> FWidgetMarkupModule::CreateCustomPropertySetter(UStruct* InStruct, FName InPropertyPath) const
{
	const FString PropertyPathString = InPropertyPath.ToString();

	if (!InStruct)
	{
		return nullptr;
	}

	FWidgetPropertyPath PropertyPath;
	FString ParseError;
	if (!FWidgetPropertyPath::TryParse(PropertyPathString, PropertyPath, &ParseError))
	{
		return nullptr;
	}

	UStruct* BestStruct = nullptr;
	const FOnCreatePropertySetter* BestDelegate = nullptr;
	for (const auto& KeyValuePair : PropertySetterCreateDelegates)
	{
		UStruct* Struct = KeyValuePair.Key.Get();
		if (!Struct)
		{
			continue;
		}
		if (!InStruct->IsChildOf(Struct))
		{
			continue;
		}

		const FOnCreatePropertySetter* Found = KeyValuePair.Value.Find(PropertyPath);
		if (!Found || !Found->IsBound())
		{
			continue;
		}

		if (!BestStruct || Struct->IsChildOf(BestStruct))
		{
			BestStruct = Struct;
			BestDelegate = Found;
		}
	}
	if (!BestDelegate)
	{
		return nullptr;
	}
	return BestDelegate->Execute();
}

bool FWidgetMarkupModule::ApplyPropertyValue(UObject* TargetObject, const FWidgetPropertyPath& PropertyPath, const FStringView& ValueString, FText* OutError) const
{
	if (!TargetObject)
	{
		if (OutError)
		{
			*OutError = FText::FromString(TEXT("Target object is null."));
		}
		return false;
	}

	if (PropertyPath.IsEmpty())
	{
		if (OutError)
		{
			*OutError = FText::FromString(TEXT("Target property path is empty."));
		}
		return false;
	}

	TSharedPtr<FPropertyChainHandle> PropertyChain = FPropertyChainHandle::Create(TargetObject, PropertyPath);
	if (!PropertyChain.IsValid())
	{
		if (OutError)
		{
			*OutError = FText::Format(
				FText::FromString(TEXT("Failed to resolve target property path '{0}' on '{1}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString()),
				FText::FromString(TargetObject->GetClass()->GetName()));
		}
		return false;
	}

	if (!PropertyChain->SetValue(ValueString))
	{
		if (OutError)
		{
			*OutError = FText::Format(
				FText::FromString(TEXT("Failed to convert or apply value to property path '{0}' on '{1}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString()),
				FText::FromString(TargetObject->GetClass()->GetName()));
		}
		return false;
	}

	return true;
}

bool FWidgetMarkupModule::ApplyPropertyValue(UObject* TargetObject, const FWidgetPropertyPath& PropertyPath, const FPropertyBuffer& PropertyBuffer, FText* OutError) const
{
	if (!TargetObject)
	{
		if (OutError) { *OutError = FText::FromString(TEXT("Target object is null.")); }
		return false;
	}

	if (PropertyPath.IsEmpty())
	{
		if (OutError) { *OutError = FText::FromString(TEXT("Target property path is empty.")); }
		return false;
	}

	TSharedPtr<FPropertyChainHandle> PropertyChain = FPropertyChainHandle::Create(TargetObject, PropertyPath);
	if (!PropertyChain.IsValid())
	{
		if (OutError)
		{
			*OutError = FText::Format(
				FText::FromString(TEXT("Failed to resolve target property path '{0}' on '{1}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString()),
				FText::FromString(TargetObject->GetClass()->GetName()));
		}
		return false;
	}

	if (!PropertyChain->SetValue(PropertyBuffer))
	{
		if (OutError)
		{
			*OutError = FText::Format(
				FText::FromString(TEXT("Failed to apply value to property path '{0}' on '{1}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString()),
				FText::FromString(TargetObject->GetClass()->GetName()));
		}
		return false;
	}

	return true;
}

UObject* FWidgetMarkupModule::CompileFromSourceCode(FName PackagePath, const FString& XML)
{
	// Normalize to long package path so object map keys are always long names.
	const FString PackagePathString = PackagePath.ToString();
	const FString LongPackagePath = PackagePathString.StartsWith(TEXT("/")) ? PackagePathString : FString::Printf(TEXT("/WidgetMarkup/%s"), *PackagePathString);
	const FName LongPackagePathName(*LongPackagePath);
	auto Package = CreatePackage(*LongPackagePath);
	Package->SetFlags(RF_Transient | RF_Public);
	Package->SetPackageFlags(PKG_InMemoryOnly);

	FText ErrorMessage;
	int32 ErrorLineNumber;
	auto WidgetTreeBuilder = MakeShared<FElementTreeBuilder>(Package);
	if (!TryParseWidgetMarkupXml(WidgetTreeBuilder.Get(), XML, ErrorMessage, ErrorLineNumber))
	{
		// Prefer the semantic error collected by the element tree builder
		// (e.g. "Property 'X': cannot use both a value and child elements");
		// fall back to the XML parse error with its line number.
		FText ErrorText;
		const FText BuilderError = WidgetTreeBuilder->GetFirstErrorText();
		if (!BuilderError.IsEmpty())
		{
			ErrorText = FText::Format(FText::FromString(TEXT("{0} (line {1})")), BuilderError, FText::AsNumber(ErrorLineNumber));
		}
		else
		{
			ErrorText = FText::Format(FText::FromString(TEXT("XML parse error at line {0}: {1}")), FText::AsNumber(ErrorLineNumber), ErrorMessage);
		}
		UE_LOG(LogWidgetMarkup, Error, TEXT("CompileFromSourceCode failed: %s"), *ErrorText.ToString());
		LastCompileErrors.Add(LongPackagePathName, ErrorText);
		return nullptr;
	}
	auto RootElementNode = WidgetTreeBuilder->GetRootElementNode();
	if (!RootElementNode.IsValid())
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("CompileFromSourceCode failed: no root element produced for '%s'."), *LongPackagePath);
		LastCompileErrors.Add(LongPackagePathName, FText::FromString(TEXT("No root element produced.")));
		return nullptr;
	}
	auto Object = RootElementNode->GetObject();
	LastCompileErrors.Remove(LongPackagePathName);
	Objects.FindOrAdd(LongPackagePathName) = Object;
	GetOnObjectCompiled().Broadcast(LongPackagePathName, Object);
	return Object;
}

UObject* FWidgetMarkupModule::CompileFromPackagePath(const FString& PackagePath)
{
	UE_LOG(LogWidgetMarkup, Display, TEXT("Compile Package Path '%s'."), *PackagePath);

	// Use the same normalized key as CompileFromSourceCode so error lookups match.
	const FString NormalizedPackagePath = PackagePath.StartsWith(TEXT("/")) ? PackagePath : FString::Printf(TEXT("/WidgetMarkup/%s"), *PackagePath);
	const FName PackagePathName(*NormalizedPackagePath);

	FString AbsoluteFilePath;
	if (!TryConvertPackagePathToAbsoluteSourceFilePath(PackagePath, FWidgetMarkupModule::SourceFileExtension, AbsoluteFilePath))
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("CompileFromPackagePath failed: invalid package path '%s' (expected /Game/... format)."), *PackagePath);
		LastCompileErrors.Add(PackagePathName, FText::FromString(TEXT("Invalid package path (expected /Game/... format).")));
		return nullptr;
	}
	FString XML;
	if (!FFileHelper::LoadFileToString(XML, *AbsoluteFilePath, FFileHelper::EHashOptions::None, FILEREAD_AllowWrite) || XML.IsEmpty())
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("CompileFromPackagePath failed: could not read file or file is empty ('%s')."), *AbsoluteFilePath);
		LastCompileErrors.Add(PackagePathName, FText::FromString(TEXT("Could not read source file or file is empty.")));
		return nullptr;
	}
	return CompileFromSourceCode(PackagePathName, XML);
}

UObject* FWidgetMarkupModule::GetObjectFromPackagePath(const FString& PackagePath)
{
	auto Object = Objects.Find(FName(PackagePath));
	return Object ? *Object : nullptr;
}

UObject* FWidgetMarkupModule::GetObjectOrCompileFromPackage(const FString& PackagePath)
{
	if (UObject* Object = GetObjectFromPackagePath(PackagePath))
	{
		return Object;
	}
	return CompileFromPackagePath(PackagePath);
}

void FWidgetMarkupModule::AddReferencedObjects(FReferenceCollector& Collector)
{
	for (auto& KeyValuePair : Objects)
	{
		Collector.AddReferencedObject(KeyValuePair.Value);
	}
}

FString FWidgetMarkupModule::GetReferencerName() const
{
	return TEXT("WidgetMarkupModule");
}

FWidgetMarkupModule::FOnObjectCompiled& FWidgetMarkupModule::GetOnObjectCompiled()
{
	return OnObjectCompiled;
}

FWidgetMarkupModule::FOnWidgetMarkupUserWidgetInitialized& FWidgetMarkupModule::GetOnWidgetMarkupUserWidgetInitialized()
{
	return OnWidgetMarkupUserWidgetInitialized;
}

bool FWidgetMarkupModule::RegisterCustomPropertyRun(UStruct* InStruct, FName InPropertyPath, FOnCreatePropertyRun InOnCreatePropertyRun)
{
	const FString PropertyPathString = InPropertyPath.ToString();

	FWidgetPropertyPath PropertyPath;
	FString ParseError;
	if (!FWidgetPropertyPath::TryParse(PropertyPathString, PropertyPath, &ParseError))
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("RegisterCustomProperty failed: invalid PropertyPath '%s' for Struct '%s': %s"), *PropertyPathString, *InStruct->GetName(), *ParseError);
		return false;
	}
	auto& Registry = PropertyRunCreateDelegates.FindOrAdd(InStruct);
	if (Registry.Contains(PropertyPath))
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("RegisterCustomProperty: overwriting existing entry (Struct='%s', PropertyPath='%s')."), *InStruct->GetName(), *PropertyPath.GetPathName().ToString());
	}
	Registry.Add(PropertyPath, InOnCreatePropertyRun);
	return true;
}

bool FWidgetMarkupModule::RegisterCustomPropertySetter(UStruct* InStruct, FName InPropertyPath, FOnCreatePropertySetter InOnCreatePropertySetter)
{
	const FString PropertyPathString = InPropertyPath.ToString();

	FWidgetPropertyPath PropertyPath;
	FString ParseError;
	if (!FWidgetPropertyPath::TryParse(PropertyPathString, PropertyPath, &ParseError))
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("RegisterCustomPropertySetter failed: invalid PropertyPath '%s' for Struct '%s': %s"), *PropertyPathString, *InStruct->GetName(), *ParseError);
		return false;
	}
	auto& Registry = PropertySetterCreateDelegates.FindOrAdd(InStruct);
	if (Registry.Contains(PropertyPath))
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("RegisterCustomPropertySetter: overwriting existing entry (Struct='%s', PropertyPath='%s')."), *InStruct->GetName(), *PropertyPath.GetPathName().ToString());
	}
	Registry.Add(PropertyPath, InOnCreatePropertySetter);
	return true;
}

void FWidgetMarkupModule::UnregisterCustomPropertyRun(UStruct* InStruct, FName InPropertyPath)
{
	const FString PropertyPathString = InPropertyPath.ToString();

	if (!InStruct)
	{
		return;
	}

	FWidgetPropertyPath PropertyPath;
	FString ParseError;
	if (!FWidgetPropertyPath::TryParse(PropertyPathString, PropertyPath, &ParseError))
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("UnregisterCustomProperty ignored invalid PropertyPath '%s' for Struct '%s': %s"), *PropertyPathString, *InStruct->GetName(), *ParseError);
		return;
	}
	auto* Registry = PropertyRunCreateDelegates.Find(InStruct);
	if (Registry)
	{
		Registry->Remove(PropertyPath);
		if (Registry->IsEmpty())
		{
			PropertyRunCreateDelegates.Remove(InStruct);
		}
	}
}

void FWidgetMarkupModule::UnregisterCustomPropertySetter(UStruct* InStruct, FName InPropertyPath)
{
	const FString PropertyPathString = InPropertyPath.ToString();

	if (!InStruct)
	{
		return;
	}

	FWidgetPropertyPath PropertyPath;
	FString ParseError;
	if (!FWidgetPropertyPath::TryParse(PropertyPathString, PropertyPath, &ParseError))
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("UnregisterCustomPropertySetter ignored invalid PropertyPath '%s' for Struct '%s': %s"), *PropertyPathString, *InStruct->GetName(), *ParseError);
		return;
	}
	auto* Registry = PropertySetterCreateDelegates.Find(InStruct);
	if (Registry)
	{
		Registry->Remove(PropertyPath);
		if (Registry->IsEmpty())
		{
			PropertySetterCreateDelegates.Remove(InStruct);
		}
	}
}

void FWidgetMarkupModule::OnPostEngineInit()
{
	EnsureSourceFileWatching();
	EnsureRemoteControlPreset();
}

void FWidgetMarkupModule::EnsureRemoteControlPreset()
{
	if (AttributePreset)
	{
		return;
	}

	// The preset layout registers undo/redo listeners on GEditor->Trans
	// unconditionally. The standalone app creates a UEditorEngine (GIsEditor is
	// true for commandlets with WITH_EDITORONLY_DATA) but not its transaction
	// buffer, so create the buffer before exposing anything.
#if WITH_EDITOR
	if (GEditor && !GEditor->Trans)
	{
		GEditor->Trans = NewObject<UTransBuffer>(GEditor, TEXT("Trans"), RF_Transactional);
	}
#endif

	// Web Remote Control only runs in the editor, but the preset machinery is
	// harmless to skip anywhere the RemoteControl plugin is unavailable.
	if (!FModuleManager::Get().LoadModule(TEXT("RemoteControl")))
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("RemoteControl module is unavailable; attribute discovery will not be exposed over HTTP."));
		return;
	}

	IRemoteControlModule& RemoteControlModule = IRemoteControlModule::Get();

	URemoteControlPreset* Preset = RemoteControlModule.CreateTransientPreset();
	if (!Preset)
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("Could not create a transient Remote Control preset."));
		return;
	}
	// The object name doubles as the preset name in Web Remote Control URLs.
	Preset->Rename(TEXT("WidgetMarkup"), nullptr, REN_NonTransactional);

	// Bind the stateless attribute library through its class default object.
	// The Web Remote Control function-call route serializes struct return
	// values into the "ReturnedValues" response array, so the struct-returning
	// functions can be exposed directly. Explicit labels keep the URL path
	// segments predictable (no spaces to URL-encode).
	UClass* LibraryClass = UWidgetMarkupLibrary::StaticClass();
	Preset->ExposeFunction(LibraryClass->GetDefaultObject(), LibraryClass->FindFunctionByName(TEXT("GetElements")), FRemoteControlPresetExposeArgs(TEXT("GetElements"), FGuid()));
	Preset->ExposeFunction(LibraryClass->GetDefaultObject(), LibraryClass->FindFunctionByName(TEXT("GetAttributes")), FRemoteControlPresetExposeArgs(TEXT("GetAttributes"), FGuid()));

	if (!RemoteControlModule.RegisterEmbeddedPreset(Preset, /*bReplaceExisting=*/ true))
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("Could not register the WidgetMarkup Remote Control preset."));
		RemoteControlModule.DestroyTransientPreset(Preset->GetPresetId());
		return;
	}

	AttributePreset = Preset;
	UE_LOG(LogWidgetMarkup, Display, TEXT("Registered Remote Control preset 'WidgetMarkup' with GetElements and GetAttributes."));

	// Verify the preset is resolvable through the embedded preset registry,
	// which is exactly what the Web Remote Control HTTP handlers consult.
	{
		TArray<TWeakObjectPtr<URemoteControlPreset>> EmbeddedPresets;
		IRemoteControlModule::Get().GetEmbeddedPresets(EmbeddedPresets);
		bool bFound = false;
		for (const TWeakObjectPtr<URemoteControlPreset>& EmbeddedPreset : EmbeddedPresets)
		{
			if (EmbeddedPreset.Get() == Preset)
			{
				bFound = true;
				break;
			}
		}
		UE_LOG(LogWidgetMarkup, Display, TEXT("Remote Control preset 'WidgetMarkup' %s in the embedded preset registry."), bFound ? TEXT("is present") : TEXT("IS MISSING"));
	}
}

void FWidgetMarkupModule::EnsureSourceFileWatching()
{
	for (const FDirectoryPath& DirectoryPath : UWidgetMarkupSettings::Get().SourceFileDirectoryPaths)
	{
		StartSourceFileWatching(DirectoryPath);
	}
}

void FWidgetMarkupModule::StartSourceFileWatching(const FDirectoryPath& InDirectoryPath)
{
	if (InDirectoryPath.Path.IsEmpty())
	{
		return;
	}
	// Convert the package directory path to an absolute disk path.
	// FPackageName supports all mounted content roots (/Game/, /PluginName/, etc.).
	FString DirectoryPath;
	if (!FPackageName::TryConvertLongPackageNameToFilename(InDirectoryPath.Path + TEXT("/"), DirectoryPath))
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("StartSourceFileWatching: could not convert '%s' to a disk path, skipping."), *InDirectoryPath.Path);
		return;
	}
	FPaths::NormalizeDirectoryName(DirectoryPath);
	DirectoryPath = FPaths::ConvertRelativePathToFull(DirectoryPath);
	if (!FPaths::DirectoryExists(DirectoryPath))
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("StartSourceFileWatching: directory does not exist '%s', skipping."), *DirectoryPath);
		return;
	}
	if (WatchedDirectories.Contains(DirectoryPath))
	{
		return;
	}
	auto& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(FName("DirectoryWatcher"));
	if (auto DirectoryWatcher = DirectoryWatcherModule.Get())
	{
		FDelegateHandle Handle;
		if (DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(
			DirectoryPath,
			IDirectoryWatcher::FDirectoryChanged::CreateLambda([this, DirectoryPath](const TArray<FFileChangeData>& FileChanges)
			{
				HandleOnSourceFileDirectoryChanged(FileChanges, DirectoryPath);
			}),
			Handle
		))
		{
			WatchedDirectories.Add(DirectoryPath, Handle);
			UE_LOG(LogWidgetMarkup, Display, TEXT("Start Source File Watching for '%s'."), *DirectoryPath);
		}
	}
}

void FWidgetMarkupModule::StopSourceFileWatching()
{
	if (WatchedDirectories.IsEmpty())
	{
		return;
	}
	if (auto DirectoryWatcherModule = FModuleManager::GetModulePtr<FDirectoryWatcherModule>(FName("DirectoryWatcher")))
	{
		if (auto DirectoryWatcher = DirectoryWatcherModule->Get())
		{
			for (auto& Pair : WatchedDirectories)
			{
				DirectoryWatcher->UnregisterDirectoryChangedCallback_Handle(Pair.Key, Pair.Value);
				UE_LOG(LogWidgetMarkup, Display, TEXT("Stop Source File Watching for '%s'."), *Pair.Key);
			}
		}
	}
	WatchedDirectories.Empty();
}

void FWidgetMarkupModule::HandleOnSourceFileDirectoryChanged(const TArray<struct FFileChangeData>& FileChanges, const FString& WatchedDirectory)
{
	UE_LOG(LogWidgetMarkup, Display, TEXT("Source File Changed: directory watcher fired with %d change(s) in '%s'."), FileChanges.Num(), *WatchedDirectory);
	for (const auto& FileChangeData : FileChanges)
	{
		// Resolve to a canonical absolute path once for all actions.
		FString AbsoluteFilePath = FileChangeData.Filename;
		if (FPaths::IsRelative(AbsoluteFilePath))
		{
			// The DirectoryWatcher may report a path relative to BaseDir()
			// (e.g. ../../../Game/Content/File.widgetmarkup) instead of
			// relative to the registered directory. ConvertRelativePathToFull
			// resolves ../ against FPlatformProcess::BaseDir(), which
			// produces the correct absolute path for both Engine/Binaries
			// and Game/Binaries base directories.
			AbsoluteFilePath = FPaths::ConvertRelativePathToFull(AbsoluteFilePath);
		}
		FPaths::NormalizeFilename(AbsoluteFilePath);

		// Skip non-widgetmarkup files (e.g. __pycache__/*.pyc, *.py) to
		// avoid spurious "could not convert to package path" warnings.
		if (!AbsoluteFilePath.EndsWith(FWidgetMarkupModule::SourceFileExtension))
		{
			continue;
		}

		FString PackagePath;
		if (!TryConvertAbsoluteSourceFilePathToPackagePath(AbsoluteFilePath, FWidgetMarkupModule::SourceFileExtension, PackagePath))
		{
			UE_LOG(LogWidgetMarkup, Warning, TEXT("Source File Changed: could not convert to package path ('%s')."), *AbsoluteFilePath);
			continue;
		}
		const FName PackagePathName(*PackagePath);

		switch (FileChangeData.Action)
		{
		case FFileChangeData::FCA_Added:
		case FFileChangeData::FCA_Modified:
		case FFileChangeData::FCA_RescanRequired:
			UE_LOG(LogWidgetMarkup, Display, TEXT("Source File Changed: '%s' -> '%s'."), *AbsoluteFilePath, *PackagePath);
			// Coalesce bursts of events (e.g. atomic saves emit several per
			// save) into a single recompile per package after a short window.
			PendingCompilePaths.Add(PackagePathName);
			EnsureCompileDebounceTicker();
			break;
		case FFileChangeData::FCA_Removed:
		{
			// A removed file needs no recompile; drop any pending one so a
			// quick delete + recreate still compiles the new content once.
			PendingCompilePaths.Remove(PackagePathName);

			// Drop the compiled object so a deleted source does not keep a
			// stale asset alive, and notify listeners (preview windows).
			if (Objects.Remove(PackagePathName) > 0)
			{
				UE_LOG(LogWidgetMarkup, Display, TEXT("Source File Removed: dropped compiled object for '%s'."), *PackagePath);
				GetOnObjectCompiled().Broadcast(PackagePathName, nullptr);
			}
			break;
		}
		case FFileChangeData::FCA_Unknown:
			break;
		default:
			ensureMsgf(false, TEXT("Not implemented for File Change Action: %d!"), FileChangeData.Action);
		}
	}
}

void FWidgetMarkupModule::EnsureCompileDebounceTicker()
{
	if (!CompileDebounceTickerHandle.IsValid())
	{
		CompileDebounceTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateRaw(this, &FWidgetMarkupModule::TickCompileDebounce),
			FWidgetMarkupModule::CompileDebounceDelaySeconds);
	}
}

bool FWidgetMarkupModule::TickCompileDebounce(float DeltaSeconds)
{
	CompileDebounceTickerHandle.Reset();

	TSet<FName> PathsToCompile = MoveTemp(PendingCompilePaths);
	PendingCompilePaths.Reset();

	for (const FName& PackagePathName : PathsToCompile)
	{
		CompileFromPackagePath(PackagePathName.ToString());
	}

	return false; // one-shot ticker
}

void FWidgetMarkupModule::StartUp(TSharedRef<IWidgetMarkupScriptIntegration> InScriptIntegration)
{
	check(!ScriptIntegration);
	ScriptIntegration = InScriptIntegration;
}

void FWidgetMarkupModule::Shutdown()
{
	bInitialized = false;
	PendingInitializedCallbacks.Empty();
	ScriptIntegration = nullptr;
}

bool FWidgetMarkupModule::IsTestMode() const
{
	// The runner passes the whole switch bundle, e.g.
	// "test -nullrhi -WidgetMarkupTestTimeout=120", so match the token.
	TArray<FString> Tokens;
	ExtraArguments.ParseIntoArrayWS(Tokens);
	return Tokens.Contains(TEXT("test"));
}

void FWidgetMarkupModule::NotifyInitialized()
{
	if (bInitialized)
	{
		return;
	}

	bInitialized = true;
	UE_LOG(LogWidgetMarkup, Display, TEXT("FWidgetMarkupModule is now initialized. Broadcasting OnInitialized and executing %d pending callbacks."), PendingInitializedCallbacks.Num());
	OnInitialized.Broadcast();

	for (FSimpleDelegate& Callback : PendingInitializedCallbacks)
	{
		Callback.ExecuteIfBound();
	}
	PendingInitializedCallbacks.Empty();
}

void FWidgetMarkupModule::ExecuteOrRegisterOnInitialized(FSimpleDelegate InCallback)
{
	if (!InCallback.IsBound())
	{
		return;
	}

	if (bInitialized)
	{
		InCallback.Execute();
		return;
	}

	PendingInitializedCallbacks.Add(MoveTemp(InCallback));
}

IMPLEMENT_MODULE(FWidgetMarkupModule, WidgetMarkup)enum class ESlotCapacityPolicy
{
	None = 0,
	Single = 1,
	Multiple = 2,
};

struct FIntelliSenseDataGenerator
{
	FString GenerateBasicTypeIntelliSenseJsonObject(const FProperty* Property)
	{
		auto TypeName = Property->GetCPPType();
		if (!BasicTypeNames.Contains(TypeName))
		{
			BasicTypeNames.Add(TypeName);
			auto Data = MakeShared<FJsonObject>();
			Data->SetStringField(TEXT("name"), TypeName);
			TArray<TSharedPtr<FJsonValue>> Properties;
			Data->SetArrayField(TEXT("properties"), Properties);
			Types.Add(MakeShared<FJsonValueObject>(Data));
			if (TypeName == TEXT("float") || TypeName == TEXT("double"))
			{
				Data->SetStringField(TEXT("pattern"), TEXT("^[+-]?(?:(\\d+\\.?\\d*|\\.\\d+)(?:[Ee][+-]?\\d+)?|INF|NaN)$"));
			}
			else if (TypeName == TEXT("int"))
			{
				Data->SetStringField(TEXT("pattern"), TEXT("^[+-]?\\d+$"));
			}
			else
			{
				UE_LOG(LogWidgetMarkup, Warning, TEXT("No regex pattern defined for type %s!"), *TypeName);
			}
		}
		return TypeName;
	}

	FString GenerateEnumIntelliSenseJsonObject(const FEnumProperty* EnumProperty)
	{
		check(EnumProperty);
		auto Enum = EnumProperty->GetEnum();
		check(Enum);
		if (EnumClasses.Contains(Enum))
		{
			return Enum->GetName();
		}
		EnumClasses.Add(Enum);
		auto Data = MakeShared<FJsonObject>();
		Data->SetStringField(TEXT("name"), Enum->GetName());
		TArray<TSharedPtr<FJsonValue>> Properties;
		Data->SetArrayField(TEXT("properties"), Properties);
		Types.Add(MakeShared<FJsonValueObject>(Data));
		TArray<TSharedPtr<FJsonValue>> Choices;
		for (int32 Index = 0; Index < Enum->NumEnums(); Index++)
		{
			auto EnumValue = Enum->GetValueByIndex(Index);
			if (EnumValue == Enum->GetMaxEnumValue())
			{
				continue;
			}
			auto Name = Enum->GetNameStringByValue(EnumValue);
			Choices.Add(MakeShared<FJsonValueString>(Name));
		}
		Data->SetArrayField(TEXT("choices"), Choices);
		return Enum->GetName();
	}

	FString GenerateObjectOrStructIntelliSenseJsonObject(const UStruct* StructOrClass)
	{
		check(StructOrClass);
		if (StructOrClasses.Contains(StructOrClass))
		{
			return StructOrClass->GetName();
		}
		StructOrClasses.Add(StructOrClass);
		auto Data = MakeShared<FJsonObject>();
		Data->SetStringField(TEXT("name"), StructOrClass->GetName());
		if (auto Class = Cast<UClass>(StructOrClass))
		{
			if (auto SuperClass = Class->GetSuperClass())
			{
				GenerateObjectOrStructIntelliSenseJsonObject(SuperClass);
				Data->SetStringField(TEXT("super"), SuperClass->GetName());
			}
			if (!(Class->ClassFlags & CLASS_Abstract))
			{
				if (Class->IsChildOf(UWidget::StaticClass()))
				{
					if (Class->IsChildOf(UPanelWidget::StaticClass()))
					{
						auto TemporaryPanelWidget = NewObject<UPanelWidget>(GetTransientPackage(), Class);
						auto TemporaryChild = NewObject<UImage>(TemporaryPanelWidget);
						auto TemporarySlot = TemporaryPanelWidget->AddChild(TemporaryChild);
						auto SlotClass = TemporarySlot->GetClass();
						GenerateObjectOrStructIntelliSenseJsonObject(SlotClass);
						Data->SetStringField(TEXT("slotClass"), SlotClass->GetName());
						if (TemporaryPanelWidget->CanHaveMultipleChildren())
						{
							Data->SetNumberField(TEXT("slotCapacityPolicy"), (int32)ESlotCapacityPolicy::Multiple);
						}
						else
						{
							Data->SetNumberField(TEXT("slotCapacityPolicy"), (int32)ESlotCapacityPolicy::Single);
						}
					}
					else
					{
						Data->SetNumberField(TEXT("slotCapacityPolicy"), (int32)ESlotCapacityPolicy::None);
					}
				}
			}
		}
		TArray<TSharedPtr<FJsonValue>> Properties;
		for (TFieldIterator<FProperty> PropertyIt(StructOrClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
		{
			auto Property = *PropertyIt;
			FString Name = Property->GetName();
			FString Type;
			if (auto ObjectProperty = CastField<FObjectProperty>(Property))
			{
				Type = GenerateObjectOrStructIntelliSenseJsonObject(ObjectProperty->PropertyClass);
			}
			else if (auto StructProperty = CastField<FStructProperty>(Property))
			{
				Type = GenerateObjectOrStructIntelliSenseJsonObject(StructProperty->Struct);
			}
			else if (auto EnumProperty = CastField<FEnumProperty>(Property))
			{
				Type = GenerateEnumIntelliSenseJsonObject(EnumProperty);
			}
			else
			{
				Type = GenerateBasicTypeIntelliSenseJsonObject(Property);
			}
			auto PropertyData = MakeShared<FJsonObject>();
			PropertyData->SetStringField(TEXT("name"), Name);
			PropertyData->SetStringField(TEXT("type"), Type);
			PropertyData->SetBoolField(TEXT("hasSetter"), Property->HasSetter());
			Properties.Add(MakeShared<FJsonValueObject>(PropertyData));
		}
		Data->SetArrayField(TEXT("properties"), Properties);
		Types.Add(MakeShared<FJsonValueObject>(Data));
		return StructOrClass->GetName();
	}

	auto Generate()
	{
		auto Data = MakeShared<FJsonObject>();
		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			UClass* Class = *ClassIt;
			if (Class->HasAnyClassFlags(CLASS_Abstract))
			{
				continue;
			}
			if (!Class->GetPackage()->GetName().StartsWith(TEXT("/Script/UMG")))
			{
				continue;
			}
			if (Class->IsChildOf(UWidget::StaticClass()) && Class != UWidget::StaticClass() && !Class->IsChildOf(UUserWidget::StaticClass()))
			{
				GenerateObjectOrStructIntelliSenseJsonObject(Class);
			}
		}
		Data->SetArrayField(TEXT("types"), MoveTemp(Types));
		return Data;
	}

	TArray<TSharedPtr<FJsonValue>> Types;
	TSet<FString> BasicTypeNames;
	TSet<const UStruct*> StructOrClasses;
	TSet<UEnum*> EnumClasses;
};

static FAutoConsoleCommand GWidgetMarkupGenerateIntelliSenseData
(
	TEXT("WidgetMarkup.GenerateIntellisenseData"),
	TEXT("Generate Intellisense Data."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			return;
		}
		auto FilePath = Args[0];
		FIntelliSenseDataGenerator Generator;
		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(Generator.Generate(), Writer);
		if (FPaths::IsRelative(FilePath))
		{
			FilePath = *FPaths::ProjectSavedDir() / FilePath;
		}
		FFileHelper::SaveStringToFile(OutputString, *FilePath);
	})
);