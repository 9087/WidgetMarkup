// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "Utilities/WidgetPropertyPath.h"

class FProperty;
class UStruct;

struct FPropertyPathResolver
{
public:
	struct FInitialState
	{
		FInitialState(void* InContainer, UStruct* InStruct, FProperty* InProperty = nullptr)
			: Property(InProperty)
			, Container(InContainer)
			, Struct(InStruct)
		{
		}

		FProperty* Property;
		void* Container;
		UStruct* Struct;
	};

	struct FOutput
	{
		FProperty* Property = nullptr;
		void* Container = nullptr;
		void* ValueAddress = nullptr;
	};

	/**
	 * Resolves a property path. With bTypeOnly=false (default) a container is
	 * required and the full value address is resolved. With bTypeOnly=true the
	 * container may be null: only the tail property type is resolved (for
	 * compile-time use without an instance). Array index and object-pointer
	 * segments cannot be resolved in this mode and fail.
	 */
	static TSharedPtr<FOutput> TryResolvePath(const FInitialState& InitialState, const FWidgetPropertyPath& Path, bool bTypeOnly = false);
};
