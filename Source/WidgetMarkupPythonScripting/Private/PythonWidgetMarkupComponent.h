// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/IWidgetMarkupComponent.h"

class UUserWidget;

class FPythonWidgetMarkupComponent : public IWidgetMarkupComponent
{
public:
	static TSharedPtr<FPythonWidgetMarkupComponent> Create(UUserWidget* InUserWidget, const FString& InScript);

	virtual ~FPythonWidgetMarkupComponent() override;
	virtual void OnDataRefresh(UObject* Data) override;

	void* GetPythonInstance() const { return PythonComponentInstance; }

private:
	void* PythonComponentInstance = nullptr;
};
