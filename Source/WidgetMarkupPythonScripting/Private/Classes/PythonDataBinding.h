// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

typedef struct _object PyObject;

bool RegisterPythonDataBindingType(PyObject* Module);
