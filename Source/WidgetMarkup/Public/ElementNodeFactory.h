// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FElementNode;

class FElementNodeFactory
{
public:
	static FElementNodeFactory& Get();

	DECLARE_DELEGATE_RetVal(TSharedRef<FElementNode>, FOnCreateElementNode)

	struct FRegisterOptions
	{
		TOptional<FString> Alias;
	};

	bool Register(UStruct* Struct, const FOnCreateElementNode& OnCreateElementNode, const FRegisterOptions& RegisterOptions = FRegisterOptions());
	bool Unregister(UStruct* Struct);

	template <typename T>
	bool Register(FOnCreateElementNode OnCreateElementNode, const FRegisterOptions& RegisterOptions = FRegisterOptions())
	{
		if constexpr (TIsDerivedFrom<T, UObject>::Value)
		{
			return Register(T::StaticClass(), OnCreateElementNode, RegisterOptions);
		}
		else
		{
			return Register(T::StaticStruct(), OnCreateElementNode, RegisterOptions);
		}
	}

	template <typename T>
	bool Unregister()
	{
		if constexpr (TIsDerivedFrom<T, UObject>::Value)
		{
			return Unregister(T::StaticClass());
		}
		else
		{
			return Unregister(T::StaticStruct());
		}
	}

	TSharedPtr<FElementNode> CreateElementNode(UObject* Outer, const FString& ElementName, const TCHAR* ElementData, UStruct*& Struct);

	/**
	 * Enumerates the markup element names registered with the factory as
	 * (ElementName, UStruct*) pairs. Aliased registrations use their alias;
	 * non-aliased non-abstract registrations use the struct/class name.
	 * Abstract base registrations (e.g. UWidget) are creator dispatchers and
	 * are skipped.
	 */
	void GetRegisteredElements(TArray<TPair<FString, UStruct*>>& OutElements) const;

private:
	UStruct* ResolveStructByAlias(const FString& ElementName);

	TMap<TWeakObjectPtr<UStruct>, FOnCreateElementNode> CreateElementNodeDelegateMap;
	TMap<FString, TWeakObjectPtr<UStruct>> AliasToStructMap;
};
