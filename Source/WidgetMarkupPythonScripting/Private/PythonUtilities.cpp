// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "PythonUtilities.h"

#include "IPythonScriptPlugin.h"

FString FPythonUtilities::PythonObjectToString(PyObject* InObject)
{
#if defined(WITH_PYTHON) && WITH_PYTHON
	if (!InObject)
	{
		return TEXT("<null>");
	}

	PyObject* PyStringObject = PyObject_Str(InObject);
	if (!PyStringObject)
	{
		return TEXT("<failed to stringify python object>");
	}

	const char* Utf8Value = PyUnicode_AsUTF8(PyStringObject);
	const FString Result = Utf8Value ? UTF8_TO_TCHAR(Utf8Value) : TEXT("<non-utf8 python string>");
	Py_DECREF(PyStringObject);
	return Result;
#else
	return InObject ? TEXT("<python unavailable>" ) : TEXT("<null>");
#endif
}

FString FPythonUtilities::ConsumePythonErrorMessage()
{
#if defined(WITH_PYTHON) && WITH_PYTHON
	PyObject* ErrorType = nullptr;
	PyObject* ErrorValue = nullptr;
	PyObject* ErrorTraceback = nullptr;
	PyErr_Fetch(&ErrorType, &ErrorValue, &ErrorTraceback);
	PyErr_NormalizeException(&ErrorType, &ErrorValue, &ErrorTraceback);

	FString ErrorMessage = FPythonUtilities::PythonObjectToString(ErrorValue ? ErrorValue : ErrorType);

	Py_XDECREF(ErrorType);
	Py_XDECREF(ErrorValue);
	Py_XDECREF(ErrorTraceback);

	if (ErrorMessage.IsEmpty())
	{
		ErrorMessage = TEXT("Unknown Python error.");
	}

	return ErrorMessage;
#else
	return TEXT("Python is not available.");
#endif
}

#if defined(WITH_PYTHON) && WITH_PYTHON
namespace
{
	PyObject* CreateStaticMethodType(const char* QualifiedName, PyMethodDef* Methods, const char* Doc)
	{
		PyType_Slot TypeSlots[] = {
			{ Py_tp_methods, Methods },
			{ Py_tp_doc, const_cast<char*>(Doc) },
			{ 0, nullptr },
		};

		PyType_Spec TypeSpec = {
			QualifiedName,
			0,
			0,
			Py_TPFLAGS_DEFAULT,
			TypeSlots,
		};

		return PyType_FromSpec(&TypeSpec);
	}
}

bool FPythonUtilities::AddStaticMethodType(
	PyObject* Module,
	const char* AttributeName,
	const char* QualifiedName,
	PyMethodDef* Methods,
	const char* Doc)
{
	FPythonAutoRelease TypeObject(CreateStaticMethodType(QualifiedName, Methods, Doc));
	if (!TypeObject)
	{
		return false;
	}

	return PyModule_AddObject(Module, AttributeName, TypeObject.Get()) == 0;
}
#endif

FPythonAutoRelease::FPythonAutoRelease(PyObject* InObject)
	: Object(InObject)
{
}

FPythonAutoRelease::~FPythonAutoRelease()
{
	Reset();
}

FPythonAutoRelease::FPythonAutoRelease(FPythonAutoRelease&& Other) noexcept
	: Object(Other.Object)
{
	Other.Object = nullptr;
}

FPythonAutoRelease& FPythonAutoRelease::operator=(FPythonAutoRelease&& Other) noexcept
{
	if (this != &Other)
	{
		Reset();
		Object = Other.Object;
		Other.Object = nullptr;
	}

	return *this;
}

PyObject* FPythonAutoRelease::Get() const
{
	return Object;
}

PyObject* FPythonAutoRelease::Release()
{
	PyObject* ReleasedObject = Object;
	Object = nullptr;
	return ReleasedObject;
}

void FPythonAutoRelease::Reset(PyObject* InObject)
{
	if (Object == InObject)
	{
		return;
	}

	#if defined(WITH_PYTHON) && WITH_PYTHON
	Py_XDECREF(Object);
	#endif
	Object = InObject;
}

FPythonAutoRelease::operator bool() const
{
	return Object != nullptr;
}

#if defined(WITH_PYTHON) && WITH_PYTHON
FPythonGILScope::FPythonGILScope()
	: GilState(PyGILState_Ensure())
{
}

FPythonGILScope::~FPythonGILScope()
{
	PyGILState_Release(GilState);
}
#endif
