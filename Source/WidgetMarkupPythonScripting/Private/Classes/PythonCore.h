// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

typedef struct _object PyObject;

bool RegisterPythonCoreType(PyObject* Module);

/** Resolve a class token to a UClass (same semantics as FClassConverter). */
class UClass;
namespace FWidgetMarkupCore
{
	UClass* ResolveClass(const FString& Token);
}
