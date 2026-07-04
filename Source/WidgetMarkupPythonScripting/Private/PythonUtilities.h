// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if defined(WITH_PYTHON) && WITH_PYTHON
THIRD_PARTY_INCLUDES_START
PRAGMA_DISABLE_REGISTER_WARNINGS
#include "Python.h"
#include "structmember.h"
PRAGMA_ENABLE_REGISTER_WARNINGS
THIRD_PARTY_INCLUDES_END
#endif

struct _object;
typedef _object PyObject;
struct PyMethodDef;

class FPythonUtilities
{
public:
	static FString PythonObjectToString(PyObject* InObject);
	static FString ConsumePythonErrorMessage();

#if defined(WITH_PYTHON) && WITH_PYTHON
	/** Register a static-method-only Python type on a module (e.g. widget_markup.DataBinding). */
	static bool AddStaticMethodType(
		PyObject* Module,
		const char* AttributeName,
		const char* QualifiedName,
		PyMethodDef* Methods,
		const char* Doc);
#endif
};

class FPythonAutoRelease
{
public:
	explicit FPythonAutoRelease(PyObject* InObject = nullptr);
	~FPythonAutoRelease();

	FPythonAutoRelease(const FPythonAutoRelease&) = delete;
	FPythonAutoRelease& operator=(const FPythonAutoRelease&) = delete;

	FPythonAutoRelease(FPythonAutoRelease&& Other) noexcept;
	FPythonAutoRelease& operator=(FPythonAutoRelease&& Other) noexcept;

	PyObject* Get() const;
	PyObject* Release();
	void Reset(PyObject* InObject = nullptr);

	explicit operator bool() const;

private:
	PyObject* Object;
};

#if defined(WITH_PYTHON) && WITH_PYTHON
class FPythonGILScope
{
public:
	FPythonGILScope();
	~FPythonGILScope();

	FPythonGILScope(const FPythonGILScope&) = delete;
	FPythonGILScope& operator=(const FPythonGILScope&) = delete;

private:
	PyGILState_STATE GilState;
};
#endif
