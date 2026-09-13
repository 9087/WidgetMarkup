// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "PropertyRun.h"

class UWidget;

/**
 * Property run for widget properties whose value does not survive instantiation.
 *
 * A widget is built once into the widget tree template and then instanced per
 * UUserWidget; the property copy in between skips transient properties that contain
 * object references (FObjectInitializer::InitProperties tests CPF_Transient together
 * with ContainsInstancedObjectProperty). UListView::ListItems is the built-in case: a
 * value written into the template never reaches an instance at all.
 *
 * The run therefore keeps the generic parsing (binding, or the property-element form
 * whose child elements are assembled into the property) and, once that has written the
 * template, snapshots the property and defers the value as a per-widget style
 * assignment. Styles are applied to every instance, so the value arrives there and goes
 * through the property setter registered for that property.
 *
 * Register it once per property, for example:
 *
 *     RegisterCustomPropertyRun(UListView::StaticClass(), TEXT("ListItems"),
 *         FOnCreatePropertyRun::CreateStatic(&FDeferredPropertyRun::Create));
 *
 * A binding needs no deferral (it is written to the instance directly), so the run
 * falls through to FPropertyRun for that case.
 */
class FDeferredPropertyRun : public FPropertyRun
{
public:
	static TSharedRef<IPropertyRun> Create();

protected:
	virtual FElementNode::FResult OnBegin(
		FElementNode::FContext& Context,
		UObject* Outer,
		const FStringView& PropertyName,
		const FStringView& PropertyValue) override;
	virtual FElementNode::FResult OnEnd(FElementNode::FContext& Context) override;

private:
	/** Property to snapshot and defer, set in OnBegin when the value is not a binding. */
	FName DeferredPropertyName;
	TWeakObjectPtr<UWidget> TargetWidget;
};
