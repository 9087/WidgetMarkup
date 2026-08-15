// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "PreviewScene.h"
#include "UObject/Object.h"
#include "UObject/StrongObjectPtr.h"

#include "WidgetMarkupWindow.generated.h"

class SWindow;
class UWidget;

DECLARE_MULTICAST_DELEGATE(FOnWidgetMarkupWindowClosed);

UCLASS(BlueprintType, Blueprintable)
class WIDGETMARKUP_API UWidgetMarkupWindow : public UObject
{
	GENERATED_BODY()

public:
	UWidgetMarkupWindow();

	UFUNCTION(BlueprintCallable, Category = "Widget Markup|Window")
	static UWidgetMarkupWindow* CreateWidgetMarkupWindow(UObject* Outer, const FString& InPackagePath);

	static bool CreateAndOpenWidgetMarkupWindow(UObject* Outer, const FString& InPackagePath, TStrongObjectPtr<UWidgetMarkupWindow>& OutWindow);

	UFUNCTION(BlueprintCallable, Category = "Widget Markup|Window")
	bool SetPackagePath(const FString& InPackagePath);

	UFUNCTION(BlueprintPure, Category = "Widget Markup|Window")
	const FString& GetPackagePath() const
	{
		return PackagePath;
	}

	UFUNCTION(BlueprintCallable, Category = "Widget Markup|Window")
	bool OpenWindow();

	UFUNCTION(BlueprintCallable, Category = "Widget Markup|Window")
	void CloseWindow();

	UFUNCTION(BlueprintPure, Category = "Widget Markup|Window")
	bool IsWindowOpen() const;

	/** Reload changed Python modules and rebuild the preview widget (bound to F5). */
	UFUNCTION(BlueprintCallable, Category = "Widget Markup|Window")
	void Refresh();

	FOnWidgetMarkupWindowClosed OnWindowClosed;

protected:
	virtual void BeginDestroy() override;

private:
	void RebuildWidget();
	void HandleOnObjectCompiled(FName Name, UObject* Object);
	void HandleSlateWindowClosed(const TSharedRef<SWindow>& ClosedWindow);
	FReply HandlePreviewWindowKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	FString PackagePath;
	TSharedPtr<FPreviewScene> PreviewScene;
	UPROPERTY(Transient)
	TObjectPtr<UWidget> Widget;
	TSharedPtr<SWindow> SlateWindow;
	bool bIsRebuilding = false;
};
