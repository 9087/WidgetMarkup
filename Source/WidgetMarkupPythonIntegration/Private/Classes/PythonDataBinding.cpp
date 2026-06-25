// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Classes/PythonDataBinding.h"

#include "Binding/WidgetPropertyBinding.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "ElementNodes/PropertyChainHandle.h"
#include "Modules/ModuleManager.h"
#include "PropertyBuffer.h"
#include "PythonUtilities.h"
#include "PythonWidgetMarkupListItem.h"
#include "Utilities/WidgetPropertyPath.h"
#include "WidgetMarkupModule.h"
#include "WidgetMarkupPythonIntegration.h"

#if defined(WITH_PYTHON) && WITH_PYTHON

namespace
{
	bool NativizePythonValueToProperty(PyObject* PyValue, FProperty* Property, void* OutData)
	{
		if (!PyValue || !Property || !OutData)
		{
			return false;
		}

		if (CastField<FIntProperty>(Property))
		{
			int32 Value = static_cast<int32>(PyLong_AsLong(PyValue));
			if (PyErr_Occurred()) { PyErr_Clear(); return false; }
			FMemory::Memcpy(OutData, &Value, sizeof(Value));
			return true;
		}
		if (CastField<FInt64Property>(Property))
		{
			int64 Value = PyLong_AsLongLong(PyValue);
			if (PyErr_Occurred()) { PyErr_Clear(); return false; }
			FMemory::Memcpy(OutData, &Value, sizeof(Value));
			return true;
		}
		if (CastField<FFloatProperty>(Property))
		{
			float Value = static_cast<float>(PyFloat_AsDouble(PyValue));
			if (PyErr_Occurred()) { PyErr_Clear(); return false; }
			FMemory::Memcpy(OutData, &Value, sizeof(Value));
			return true;
		}
		if (CastField<FDoubleProperty>(Property))
		{
			double Value = PyFloat_AsDouble(PyValue);
			if (PyErr_Occurred()) { PyErr_Clear(); return false; }
			FMemory::Memcpy(OutData, &Value, sizeof(Value));
			return true;
		}
		if (CastField<FBoolProperty>(Property))
		{
			bool Value = PyObject_IsTrue(PyValue) != 0;
			static_cast<FBoolProperty*>(Property)->SetPropertyValue(OutData, Value);
			return true;
		}
		if (CastField<FStrProperty>(Property))
		{
			if (!PyUnicode_Check(PyValue)) return false;
			const char* Utf8 = PyUnicode_AsUTF8(PyValue);
			if (!Utf8) return false;
			FString Value(UTF8_TO_TCHAR(Utf8));
			static_cast<FStrProperty*>(Property)->SetPropertyValue(OutData, Value);
			return true;
		}
		if (CastField<FTextProperty>(Property))
		{
			FString TextString;
			if (PyUnicode_Check(PyValue))
			{
				const char* Utf8 = PyUnicode_AsUTF8(PyValue);
				if (!Utf8) return false;
				TextString = UTF8_TO_TCHAR(Utf8);
			}
			else if (PyLong_Check(PyValue))
			{
				const long LongVal = PyLong_AsLong(PyValue);
				if (PyErr_Occurred()) { PyErr_Clear(); return false; }
				TextString = FString::Printf(TEXT("%ld"), LongVal);
			}
			else if (PyFloat_Check(PyValue))
			{
				const double DoubleVal = PyFloat_AsDouble(PyValue);
				if (PyErr_Occurred()) { PyErr_Clear(); return false; }
				TextString = FString::SanitizeFloat(DoubleVal);
			}
			else
			{
				return false;
			}
			FText Value = FText::FromString(TextString);
			static_cast<FTextProperty*>(Property)->SetPropertyValue(OutData, Value);
			return true;
		}
		if (CastField<FNameProperty>(Property))
		{
			if (!PyUnicode_Check(PyValue)) return false;
			const char* Utf8 = PyUnicode_AsUTF8(PyValue);
			if (!Utf8) return false;
			FName Value(UTF8_TO_TCHAR(Utf8));
			static_cast<FNameProperty*>(Property)->SetPropertyValue(OutData, Value);
			return true;
		}
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			// Recursively fill struct fields from Python dict or object attributes.
			UScriptStruct* Struct = StructProperty->Struct;
			Struct->InitializeStruct(OutData);

			for (TFieldIterator<FProperty> It(Struct); It; ++It)
			{
				FProperty* Field = *It;
				const char* FieldName = TCHAR_TO_UTF8(*Field->GetName());

				PyObject* PyField = PyDict_Check(PyValue)
					? PyDict_GetItemString(PyValue, FieldName)       // dict: key lookup
					: PyObject_GetAttrString(PyValue, FieldName);    // object: getattr

				if (!PyField)
				{
					PyErr_Clear();
					continue;
				}

				if (!NativizePythonValueToProperty(PyField, Field,
						Field->ContainerPtrToValuePtr<void>(OutData)))
					return false;
			}
			return true;
		}
		if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			if (!PyList_Check(PyValue)) return false;

			FProperty* InnerProperty = ArrayProperty->Inner;
			FScriptArrayHelper ArrayHelper(ArrayProperty, OutData);
			const Py_ssize_t ListLen = PyList_Size(PyValue);
			ArrayHelper.Resize(ListLen);
			for (Py_ssize_t Index = 0; Index < ListLen; ++Index)
			{
				PyObject* PyElement = PyList_GetItem(PyValue, Index);
				void* ElementPtr = ArrayHelper.GetRawPtr(Index);
				if (!PyElement || !ElementPtr) return false;

				if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InnerProperty))
				{
					// Always wrap arbitrary Python objects as list items;
					// no longer depends on PyConversion::NativizeObject.
					UPythonWidgetMarkupListItem* Item = UPythonWidgetMarkupListItem::Create(GetTransientPackage(), PyElement);
					ObjectProperty->SetObjectPropertyValue(ElementPtr, Item);
				}
				else if (!NativizePythonValueToProperty(PyElement, InnerProperty, ElementPtr))
				{
					return false;
				}
			}
			return true;
		}

		return false;
	}

	PyObject* PyApplyPropertyBinding(PyObject* /*Self*/, PyObject* Args)
	{
		const char* UserWidgetNameOrPath = nullptr;
		const char* SourceExpressionUtf8 = nullptr;
		const char* TargetObjectNameUtf8 = nullptr;
		const char* TargetPropertyPathUtf8 = nullptr;
		PyObject* PyValue = nullptr;
		if (!PyArg_ParseTuple(Args, "ssssO:apply_property_binding", &UserWidgetNameOrPath, &SourceExpressionUtf8, &TargetObjectNameUtf8, &TargetPropertyPathUtf8, &PyValue))
		{
			return nullptr;
		}

		UUserWidget* UserWidget = FindObject<UUserWidget>(nullptr, UTF8_TO_TCHAR(UserWidgetNameOrPath));

		FWidgetPropertyBinding Binding;
		Binding.SourceExpression = FString(UTF8_TO_TCHAR(SourceExpressionUtf8));
		Binding.TargetObjectName = FName(UTF8_TO_TCHAR(TargetObjectNameUtf8));
		Binding.TargetPropertyPath = FString(UTF8_TO_TCHAR(TargetPropertyPathUtf8));
		if (!UserWidget || !UserWidget->WidgetTree || !PyValue)
		{
			Py_RETURN_NONE;
		}

		FWidgetMarkupModule* WidgetMarkupModule = FModuleManager::GetModulePtr<FWidgetMarkupModule>(TEXT("WidgetMarkup"));
		if (!WidgetMarkupModule)
		{
			Py_RETURN_NONE;
		}

		if (Binding.SourceExpression.IsEmpty() || Binding.TargetObjectName.IsNone() || Binding.TargetPropertyPath.IsEmpty())
		{
			Py_RETURN_NONE;
		}

		UWidget* TargetWidget = UserWidget->WidgetTree->FindWidget(Binding.TargetObjectName);
		if (!TargetWidget)
		{
			UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Binding skipped because target widget '%s' was not found."), *Binding.TargetObjectName.ToString());
			Py_RETURN_NONE;
		}

		FText Error;
		FWidgetPropertyPath TargetPropertyPath;
		FString ParseError;
		if (!FWidgetPropertyPath::TryParse(FStringView(Binding.TargetPropertyPath), TargetPropertyPath, &ParseError))
		{
			UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Binding failed: SourceExpression='%s', Target='%s.%s', Reason='Invalid property path: %s'."), *Binding.SourceExpression, *Binding.TargetObjectName.ToString(), *Binding.TargetPropertyPath, *ParseError);
			Py_RETURN_NONE;
		}

		TSharedPtr<FPropertyChainHandle> PropertyChain = FPropertyChainHandle::Create(TargetWidget, TargetPropertyPath);
		if (!PropertyChain.IsValid())
		{
			UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Binding failed: SourceExpression='%s', Target='%s.%s', Reason='Could not resolve property chain on '%s'.'"),
				*Binding.SourceExpression, *Binding.TargetObjectName.ToString(), *Binding.TargetPropertyPath, *TargetWidget->GetClass()->GetName());
			Py_RETURN_NONE;
		}

		FProperty* TailProperty = PropertyChain->GetTailProperty();
		if (!TailProperty)
		{
			UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Binding failed: SourceExpression='%s', Target='%s.%s', Reason='Could not resolve terminal property on '%s'.'"),
				*Binding.SourceExpression, *Binding.TargetObjectName.ToString(), *Binding.TargetPropertyPath, *TargetWidget->GetClass()->GetName());
			Py_RETURN_NONE;
		}

		FPropertyBuffer PropertyBuffer(TailProperty);
		if (!PropertyBuffer.HasValue())
		{
			UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Binding failed: SourceExpression='%s', Target='%s.%s', Reason='Could not allocate buffer for property type '%s'.'"),
				*Binding.SourceExpression, *Binding.TargetObjectName.ToString(), *Binding.TargetPropertyPath, *TailProperty->GetClass()->GetName());
			Py_RETURN_NONE;
		}

		if (!NativizePythonValueToProperty(PyValue, TailProperty, PropertyBuffer.GetValueData()))
		{
			FString ExpectedType = TailProperty->GetClass()->GetName();
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(TailProperty))
			{
				ExpectedType += FString::Printf(TEXT(" (%s)"), *StructProperty->Struct->GetName());
			}
			const FString FallbackString = FPythonUtilities::PythonObjectToString(PyValue);
			UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Binding: SourceExpression='%s', Target='%s.%s', Python value could not be converted to '%s', falling back to string."),
				*Binding.SourceExpression, *Binding.TargetObjectName.ToString(), *Binding.TargetPropertyPath, *ExpectedType);

			FPropertyBuffer FallbackBuffer(TailProperty, FStringView(FallbackString));
			if (FallbackBuffer.HasValue())
			{
				PropertyBuffer = MoveTemp(FallbackBuffer);
			}
			else
			{
				UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Binding failed: SourceExpression='%s', Target='%s.%s', Reason='String fallback also failed.'"),
					*Binding.SourceExpression, *Binding.TargetObjectName.ToString(), *Binding.TargetPropertyPath);
				Py_RETURN_NONE;
			}
		}

		if (!WidgetMarkupModule->ApplyPropertyValue(TargetWidget, TargetPropertyPath, PropertyBuffer, &Error))
		{
			UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Binding failed: SourceExpression='%s', Target='%s.%s', Reason='%s'."), *Binding.SourceExpression, *Binding.TargetObjectName.ToString(), *Binding.TargetPropertyPath, *Error.ToString());
		}
		Py_RETURN_NONE;
	}

	PyMethodDef DataBindingMethods[] =
	{
		{ "apply_property_binding", PyApplyPropertyBinding, METH_VARARGS | METH_STATIC, "Apply one WidgetMarkup property binding with a Python value." },
		{ nullptr, nullptr, 0, nullptr }
	};
}

bool RegisterPythonDataBindingType(PyObject* Module)
{
	return FPythonUtilities::AddStaticMethodType(
		Module,
		"DataBinding",
		"widget_markup.DataBinding",
		DataBindingMethods,
		"WidgetMarkup data-binding helpers.");
}

#endif
