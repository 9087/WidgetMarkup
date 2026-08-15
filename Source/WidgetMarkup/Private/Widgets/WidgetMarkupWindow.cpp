// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Widgets/WidgetMarkupWindow.h"

#include "InputCoreTypes.h"
#include "Misc/PackageName.h"
#include "UObject/GarbageCollection.h"
#include "WidgetBlueprint.h"
#include "WidgetMarkupModule.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SWindow.h"

UWidgetMarkupWindow::UWidgetMarkupWindow() = default;

UWidgetMarkupWindow* UWidgetMarkupWindow::CreateWidgetMarkupWindow(UObject* Outer, const FString& InPackagePath)
{
	UObject* ObjectOuter = Outer ? Outer : GetTransientPackage();
	UWidgetMarkupWindow* WidgetMarkupWindow = NewObject<UWidgetMarkupWindow>(ObjectOuter);
	if (!WidgetMarkupWindow || !WidgetMarkupWindow->SetPackagePath(InPackagePath))
	{
		return nullptr;
	}
	return WidgetMarkupWindow;
}

bool UWidgetMarkupWindow::CreateAndOpenWidgetMarkupWindow(UObject* Outer, const FString& InPackagePath, TStrongObjectPtr<UWidgetMarkupWindow>& OutWindow)
{
	FText PackagePathError;
	if (!FPackageName::IsValidTextForLongPackageName(InPackagePath, &PackagePathError))
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("WidgetMarkup window: invalid package path '%s': %s"), *InPackagePath, *PackagePathError.ToString());
		return false;
	}

	UObject* ObjectOuter = Outer ? Outer : GetTransientPackage();
	UWidgetMarkupWindow* Window = NewObject<UWidgetMarkupWindow>(ObjectOuter);
	if (!Window)
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("WidgetMarkup window: failed to create UWidgetMarkupWindow."));
		return false;
	}

	// Pin before SetPackagePath/OpenWindow because those code paths can allocate
	// objects that trigger GC.
	OutWindow.Reset(Window);

	if (!Window->SetPackagePath(InPackagePath) || !Window->OpenWindow())
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("WidgetMarkup window: failed to open window for '%s'."), *InPackagePath);
		OutWindow.Reset();
		return false;
	}

	return true;
}

bool UWidgetMarkupWindow::SetPackagePath(const FString& InPackagePath)
{
	PackagePath = InPackagePath;

	auto& WidgetMarkupModule = FModuleManager::Get().LoadModuleChecked<FWidgetMarkupModule>(TEXT("WidgetMarkup"));
	WidgetMarkupModule.GetOnObjectCompiled().RemoveAll(this);
	WidgetMarkupModule.GetOnObjectCompiled().AddUObject(this, &UWidgetMarkupWindow::HandleOnObjectCompiled);

	RebuildWidget();
	return true;
}

bool UWidgetMarkupWindow::OpenWindow()
{
	if (PackagePath.IsEmpty() || !FSlateApplication::IsInitialized())
	{
		return false;
	}

	if (SlateWindow.IsValid())
	{
		SlateWindow->BringToFront(true);
		return true;
	}

	const FString AssetName = FPackageName::GetShortName(PackagePath);
	TSharedRef<SWindow> NewWindow = SNew(SWindow)
		.Title(FText::FromString(AssetName))
		[SNullWidget::NullWidget];
	NewWindow->SetOnWindowClosed(FOnWindowClosed::CreateUObject(this, &UWidgetMarkupWindow::HandleSlateWindowClosed));
	NewWindow->SetOnKeyDown(FOnKeyDown::CreateUObject(this, &UWidgetMarkupWindow::HandlePreviewWindowKeyDown));
	SlateWindow = NewWindow;
	FSlateApplication::Get().AddWindow(NewWindow);
	RebuildWidget();
	return true;
}

void UWidgetMarkupWindow::RebuildWidget()
{
	if (bIsRebuilding || !SlateWindow.IsValid())
	{
		return;
	}

	TGuardValue<bool> RebuildGuard(bIsRebuilding, true);

	const TSharedRef<SWindow> LocalWindow = SlateWindow.ToSharedRef();

	// Detach the previous widget from the member so it can be collected once
	// the new content replaces it in the window (see the swap below).
	TObjectPtr<UWidget> OldWidget = Widget;
	Widget = nullptr;

	auto& WidgetMarkupModule = FModuleManager::Get().LoadModuleChecked<FWidgetMarkupModule>(TEXT("WidgetMarkup"));
	UObject* Object = WidgetMarkupModule.GetObjectOrCompileFromPackage(PackagePath);

	TSharedPtr<SWidget> NewContent = nullptr;

	if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Object))
	{
		if (UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(WidgetBlueprint->GeneratedClass))
		{
			if (!PreviewScene)
			{
				PreviewScene = MakeShared<FPreviewScene>(
					FPreviewScene::ConstructionValues()
					.AllowAudioPlayback(true)
					.ShouldSimulatePhysics(true)
				);
			}
			Widget = CreateWidget(PreviewScene->GetWorld(), WidgetClass);
			if (Widget)
			{
				NewContent = Widget->TakeWidget();
			}
		}
	}

	if (NewContent.IsValid())
	{
		LocalWindow->SetContent(NewContent.ToSharedRef());

		if (OldWidget)
		{
			// The window no longer references the old slate tree, so break the
			// widget's self-cycle (UWidget owns its SObjectWidget, which keeps
			// the widget alive via FGCObject) and collect the old widget, its
			// Python component, and style buffers right away; the standalone
			// app loop never runs GC on its own.
			const FString OldWidgetPath = OldWidget->GetPathName();
			TWeakObjectPtr<UWidget> WeakOldWidget = OldWidget;
			OldWidget->ReleaseSlateResources(true);
			OldWidget->MarkAsGarbage();
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
			ensureMsgf(!WeakOldWidget.IsValid(), TEXT("Previous preview widget '%s' was not collected; a strong reference to it still exists."), *OldWidgetPath);
			OldWidget = nullptr;
		}

		LocalWindow->MarkPrepassAsDirty();
		LocalWindow->SlatePrepass();
		const FVector2D DesiredSize = NewContent->GetDesiredSize();
		UE_LOG(LogWidgetMarkup, Log, TEXT("WidgetMarkup window: content DesiredSize = %.0fx%.0f"), DesiredSize.X, DesiredSize.Y);
		LocalWindow->Resize(FVector2D(
			FMath::Max(DesiredSize.X, 300.0f),
			FMath::Max(DesiredSize.Y, 200.0f)
		));
		LocalWindow->MarkPrepassAsDirty();
	}
	else if (OldWidget)
	{
		// Compile or widget creation failed; keep displaying the previous
		// widget and restore the reference so a later successful rebuild can
		// still release it.
		Widget = OldWidget;
	}
}

void UWidgetMarkupWindow::CloseWindow()
{
	if (SlateWindow.IsValid())
	{
		SlateWindow->RequestDestroyWindow();
	}
	SlateWindow.Reset();
}

void UWidgetMarkupWindow::Refresh()
{
	if (FWidgetMarkupModule* WidgetMarkupModule = FModuleManager::Get().GetModulePtr<FWidgetMarkupModule>(TEXT("WidgetMarkup")))
	{
		if (const TSharedPtr<IWidgetMarkupScriptIntegration> ScriptIntegration = WidgetMarkupModule->GetScriptIntegration())
		{
			ScriptIntegration->HandleRefreshRequest();
		}
	}

	RebuildWidget();
}

FReply UWidgetMarkupWindow::HandlePreviewWindowKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::F5)
	{
		UE_LOG(LogWidgetMarkup, Display, TEXT("WidgetMarkup window: F5 refresh for '%s'."), *PackagePath);
		Refresh();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

bool UWidgetMarkupWindow::IsWindowOpen() const
{
	return SlateWindow.IsValid();
}

void UWidgetMarkupWindow::HandleSlateWindowClosed(const TSharedRef<SWindow>& ClosedWindow)
{
	if (SlateWindow == ClosedWindow)
	{
		SlateWindow.Reset();
	}

	OnWindowClosed.Broadcast();
}

void UWidgetMarkupWindow::BeginDestroy()
{
	if (FWidgetMarkupModule* WidgetMarkupModule = FModuleManager::Get().GetModulePtr<FWidgetMarkupModule>(TEXT("WidgetMarkup")))
	{
		WidgetMarkupModule->GetOnObjectCompiled().RemoveAll(this);
	}
	PreviewScene.Reset();
	CloseWindow();
	Super::BeginDestroy();
}

void UWidgetMarkupWindow::HandleOnObjectCompiled(FName Name, UObject* Object)
{
	if (Name == FName(*PackagePath))
	{
		RebuildWidget();
	}
}

static FAutoConsoleCommand GWidgetMarkupShow
(
	TEXT("WidgetMarkup.Show"),
	TEXT("Show the Widget Markup Content in a Window. Expects a package path like /Game/WidgetMarkup/MyWidget or /WidgetMarkupApp/WidgetMarkup/MyWidget (no file extension)."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			return;
		}
		const FString& InputPackagePath = Args[0];
		TStrongObjectPtr<UWidgetMarkupWindow> WindowObject;
		if (!UWidgetMarkupWindow::CreateAndOpenWidgetMarkupWindow(GetTransientPackage(), InputPackagePath, WindowObject))
		{
			UE_LOG(LogWidgetMarkup, Error, TEXT("WidgetMarkup.Show: failed to create or open window for '%s'."), *InputPackagePath);
			return;
		}
		UWidgetMarkupWindow* WindowRaw = WindowObject.Get();
		WindowRaw->AddToRoot();
		WindowRaw->OnWindowClosed.AddLambda([WindowRaw]()
		{
			WindowRaw->RemoveFromRoot();
		});
	})
);
