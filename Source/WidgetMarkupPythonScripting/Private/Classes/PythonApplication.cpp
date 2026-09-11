// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Classes/PythonApplication.h"

#include "PythonUtilities.h"
#include "WidgetMarkupModule.h"

#if defined(WITH_PYTHON) && WITH_PYTHON

namespace
{
	PyObject* PyRequestShutdown(PyObject* /*Self*/, PyObject* /*Args*/)
	{
		// Graceful exit that still carries the exit code reported through
		// Application.set_exit_code(). Plain RequestExit(false) always posts 0,
		// which makes a failing test look successful to the runner.
		const int32 ExitCode = FWidgetMarkupModule::Get().GetExitCode();
		FPlatformMisc::RequestExitWithStatus(false, static_cast<uint8>(FMath::Clamp(ExitCode, 0, 255)));
		Py_RETURN_NONE;
	}

	PyObject* PyGetExtraArguments(PyObject* /*Self*/, PyObject* /*Args*/)
	{
		return PyUnicode_FromString(TCHAR_TO_UTF8(*FWidgetMarkupModule::Get().GetExtraArguments()));
	}

	PyObject* PyIsTestMode(PyObject* /*Self*/, PyObject* /*Args*/)
	{
		if (FWidgetMarkupModule::Get().IsTestMode())
		{
			Py_RETURN_TRUE;
		}
		Py_RETURN_FALSE;
	}

	PyObject* PySetExitCode(PyObject* /*Self*/, PyObject* Args)
	{
		int32 ExitCode = 0;
		if (!PyArg_ParseTuple(Args, "i:set_exit_code", &ExitCode))
		{
			return nullptr;
		}

		FWidgetMarkupModule::Get().SetExitCode(ExitCode);
		Py_RETURN_NONE;
	}

	PyMethodDef ApplicationMethods[] =
	{
		{ "get_extra_arguments", PyGetExtraArguments, METH_NOARGS | METH_STATIC, "Get the current WidgetMarkupApp extra arguments string." },
		{ "is_test_mode", PyIsTestMode, METH_NOARGS | METH_STATIC, "True when the app was launched with the 'test' extra argument token." },
		{ "request_shutdown", PyRequestShutdown, METH_NOARGS | METH_STATIC, "Request engine exit (for standalone programs)." },
		{ "set_exit_code", PySetExitCode, METH_VARARGS | METH_STATIC, "Set the process exit code (for standalone programs and test runs)." },
		{ nullptr, nullptr, 0, nullptr }
	};
}

bool RegisterPythonApplicationType(PyObject* Module)
{
	return FPythonUtilities::AddStaticMethodType(
		Module,
		"Application",
		"_widget_markup.Application",
		ApplicationMethods,
		"WidgetMarkupApp application helpers.");
}

#endif
