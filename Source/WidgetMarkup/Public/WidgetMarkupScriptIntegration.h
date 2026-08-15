// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FWidgetMarkupModule;

class WIDGETMARKUP_API IWidgetMarkupScriptIntegration : public TSharedFromThis<IWidgetMarkupScriptIntegration>
{
protected:
	explicit IWidgetMarkupScriptIntegration(FWidgetMarkupModule& InWidgetMarkupModule);

public:
	virtual ~IWidgetMarkupScriptIntegration() = default;

	void Initialize(bool bOK);

	/** Called when a refresh is requested (e.g. F5 in a preview window). The integration should reload its changed script modules. */
	virtual void HandleRefreshRequest() {}

protected:
	FWidgetMarkupModule& WidgetMarkupModule;
};