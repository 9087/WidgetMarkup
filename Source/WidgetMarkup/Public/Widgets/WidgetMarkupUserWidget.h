// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "WidgetMarkupUserWidget.generated.h"

class UListViewBase;

/**
 * Pointer event data with UPROPERTY fields so UFUNCTION marshaling works.
 * Populated from FPointerEvent by BindOnPointerEvent's native wrapper.
 */
USTRUCT(BlueprintType, meta = (ScriptName = "WidgetMarkupPointerEvent"))
struct WIDGETMARKUP_API FWidgetMarkupPointerEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FKey EffectingButton;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	TArray<FKey> PressedButtons;

	FWidgetMarkupPointerEvent() = default;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetMarkupEntryReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetMarkupItemSelectionChanged, bool, bIsSelected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetMarkupItemExpansionChanged, bool, bIsExpanded);

/**
 * Payload UObject for pointer-event callbacks.  The delegate receives this
 * object so Python can read input fields and write the return value back.
 * Avoids the RetVal / ReturnValueOffset bug in Python's UFunction generation.
 */
UCLASS()
class UWidgetMarkupOnPointerEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FGeometry Geometry;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FWidgetMarkupPointerEvent MouseEvent;

	/** Set to the desired reply in the handler. Default is Unhandled. */
	UPROPERTY(BlueprintReadWrite, Category = "Output")
	FEventReply Reply;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FWidgetMarkupOnPointerEvent, UWidgetMarkupOnPointerEventPayload*, Payload);

/**
 * Transient delegate wrapper that bridges a Widget's native FOnPointerEvent delegate
 * to a Python-bound FWidgetMarkupOnPointerEvent.  Its UFUNCTION receives the raw
 * FPointerEvent, converts it to FWidgetMarkupPointerEvent, then forwards to
 * the held TargetDelegate via a payload UObject.
 */
UCLASS(Transient)
class UWidgetMarkupOnPointerEventDelegate : public UObject
{
	GENERATED_BODY()

public:
	FWidgetMarkupOnPointerEvent TargetDelegate;

	UFUNCTION()
	FEventReply HandlePointerEvent(FGeometry Geometry, FPointerEvent PointerEvent);
};

/**
 * Base class for WidgetMarkup ListView entry widgets.
 * Bridges the ListView entry creation to WidgetMarkupComponent.on_data_refresh.
 */
UCLASS()
class WIDGETMARKUP_API UWidgetMarkupUserWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;
	virtual void NativeOnItemExpansionChanged(bool bIsExpanded) override;

	/** Get the raw list item object assigned to this entry. */
	UFUNCTION(BlueprintCallable, Category = "WidgetMarkup")
	UObject* GetEntryListItem() const;

	/** Check if this entry is currently selected in its owning list view. */
	UFUNCTION(BlueprintCallable, Category = "WidgetMarkup")
	bool IsEntrySelected() const;

	/** Get the owning ListView for this entry widget. */
	UFUNCTION(BlueprintCallable, Category = "WidgetMarkup")
	UListViewBase* GetOwningEntryListView() const;

	/** Check whether a named delegate on a Widget has FPointerEvent in its signature. */
	UFUNCTION(BlueprintCallable, Category = "WidgetMarkup")
	static bool IsOnPointerEvent(UWidget* Widget, FName DelegateName);

	/** Bind a native wrapper that converts FPointerEvent → FWidgetMarkupPointerEvent
	 *  through reflection on the Widget's named delegate property.
	 *  @return true if the delegate property was found and bound successfully. */
	UFUNCTION(BlueprintCallable, Category = "WidgetMarkup")
	static bool BindOnPointerEvent(UWidget* Widget, FName DelegateName, FWidgetMarkupOnPointerEvent NewDelegate);

	/** Broadcast when this entry is released from the owning list view. */
	UPROPERTY(BlueprintAssignable, Category = "WidgetMarkup")
	FOnWidgetMarkupEntryReleased OnEntryReleased;

	/** Broadcast when the selection state of this entry changes. */
	UPROPERTY(BlueprintAssignable, Category = "WidgetMarkup")
	FOnWidgetMarkupItemSelectionChanged OnItemSelectionChanged;

	/** Broadcast when the expansion state of this entry changes (TreeView only). */
	UPROPERTY(BlueprintAssignable, Category = "WidgetMarkup")
	FOnWidgetMarkupItemExpansionChanged OnItemExpansionChanged;
};
