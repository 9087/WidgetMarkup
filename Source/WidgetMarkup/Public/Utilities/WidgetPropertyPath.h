// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "WidgetPropertyPath.generated.h"

UENUM()
enum class EWidgetPropertyPathElementType : uint8
{
	Property,
	ArrayIndex,
};

USTRUCT()
struct WIDGETMARKUP_API FWidgetPropertyPathElement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "PropertyPath")
	EWidgetPropertyPathElementType Type = EWidgetPropertyPathElementType::Property;

	UPROPERTY(EditAnywhere, Category = "PropertyPath")
	FString Name;

	UPROPERTY(EditAnywhere, Category = "PropertyPath")
	int32 ArrayIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "PropertyPath")
	bool bIsAny = false;

	static FWidgetPropertyPathElement MakeProperty(const FStringView& PropertyName);
	static FWidgetPropertyPathElement MakeAnyProperty();
	static FWidgetPropertyPathElement MakeArrayIndex(int32 InArrayIndex);
	static FWidgetPropertyPathElement MakeAnyArrayIndex();

	bool Matches(const FWidgetPropertyPathElement& Candidate) const;
	FString ToString() const;
};

USTRUCT()
struct WIDGETMARKUP_API FWidgetPropertyPath
{
	GENERATED_BODY()

public:
	FWidgetPropertyPath() = default;
	explicit FWidgetPropertyPath(const FStringView& InText);
	explicit FWidgetPropertyPath(const TCHAR* InText);

	static bool TryParse(const FStringView& InText, FWidgetPropertyPath& OutPath, FString* OutError = nullptr);

	void Reset();
	bool IsEmpty() const;
	bool HasAny() const;
	const FName& GetPathName() const;
	bool operator==(const FWidgetPropertyPath& Other) const;
	bool Matches(const FWidgetPropertyPath& Candidate) const;
	bool TryMakeRelativeTo(const FWidgetPropertyPath& BasePath, FWidgetPropertyPath& OutRelativePath) const;

	FWidgetPropertyPath WithAppendedProperty(const FStringView& PropertyPathString) const;
	FWidgetPropertyPath WithAppendedAnyProperty() const;
	FWidgetPropertyPath WithAppendedArrayIndex(int32 ArrayIndex) const;
	FWidgetPropertyPath WithAppendedAnyArrayIndex() const;

	const TArray<FWidgetPropertyPathElement>& GetElements() const;

	friend uint32 GetTypeHash(const FWidgetPropertyPath& PropertyPath)
	{
		return GetTypeHash(PropertyPath.GetPathName());
	}

private:
	void AppendProperty(const FStringView& PropertyName);
	void AppendAnyProperty();
	void AppendArrayIndex(int32 ArrayIndex);
	void AppendAnyArrayIndex();
	void MarkPathNameDirty();

	UPROPERTY(EditAnywhere, Category = "PropertyPath")
	TArray<FWidgetPropertyPathElement> Elements;

	UPROPERTY(VisibleAnywhere, Transient, Category = "PropertyPath")
	mutable FName PathName;

	mutable bool bIsPathNameDirty = true;
};