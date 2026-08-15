// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Utilities/WidgetPropertyPath.h"

namespace
{
	static bool TryParseArrayIndexToken(const FString& Token, int32& OutArrayIndex)
	{
		if (!LexTryParseString(OutArrayIndex, *Token))
		{
			return false;
		}

		return OutArrayIndex >= 0;
	}

	static bool IsValidPropertyToken(const FString& Token)
	{
		return !Token.IsEmpty() && !Token.Contains(TEXT(".")) && !Token.Contains(TEXT("[")) && !Token.Contains(TEXT("]"));
	}

	static bool AreElementsEqual(const FWidgetPropertyPathElement& Left, const FWidgetPropertyPathElement& Right)
	{
		if (Left.Type != Right.Type || Left.bIsAny != Right.bIsAny)
		{
			return false;
		}

		switch (Left.Type)
		{
		case EWidgetPropertyPathElementType::Property:
			return Left.Name.Equals(Right.Name, ESearchCase::CaseSensitive);

		case EWidgetPropertyPathElementType::ArrayIndex:
			return Left.ArrayIndex == Right.ArrayIndex;

		default:
			return false;
		}
	}
}

FWidgetPropertyPathElement FWidgetPropertyPathElement::MakeProperty(const FStringView& PropertyName)
{
	FWidgetPropertyPathElement Element;
	Element.Type = EWidgetPropertyPathElementType::Property;
	Element.Name = FString(PropertyName);
	return Element;
}

FWidgetPropertyPathElement FWidgetPropertyPathElement::MakeAnyProperty()
{
	FWidgetPropertyPathElement Element;
	Element.Type = EWidgetPropertyPathElementType::Property;
	Element.bIsAny = true;
	return Element;
}

FWidgetPropertyPathElement FWidgetPropertyPathElement::MakeArrayIndex(int32 InArrayIndex)
{
	FWidgetPropertyPathElement Element;
	Element.Type = EWidgetPropertyPathElementType::ArrayIndex;
	Element.ArrayIndex = InArrayIndex;
	return Element;
}

FWidgetPropertyPathElement FWidgetPropertyPathElement::MakeAnyArrayIndex()
{
	FWidgetPropertyPathElement Element;
	Element.Type = EWidgetPropertyPathElementType::ArrayIndex;
	Element.bIsAny = true;
	return Element;
}

bool FWidgetPropertyPathElement::Matches(const FWidgetPropertyPathElement& Candidate) const
{
	if (Type != Candidate.Type)
	{
		return false;
	}

	if (bIsAny)
	{
		return true;
	}

	switch (Type)
	{
	case EWidgetPropertyPathElementType::Property:
		return Name.Equals(Candidate.Name, ESearchCase::CaseSensitive);

	case EWidgetPropertyPathElementType::ArrayIndex:
		return ArrayIndex == Candidate.ArrayIndex;

	default:
		return false;
	}
}

FString FWidgetPropertyPathElement::ToString() const
{
	switch (Type)
	{
	case EWidgetPropertyPathElementType::Property:
		return bIsAny ? TEXT("*") : Name;

	case EWidgetPropertyPathElementType::ArrayIndex:
		return bIsAny ? TEXT("[*]") : FString::Printf(TEXT("[%d]"), ArrayIndex);

	default:
		return FString();
	}
}

FWidgetPropertyPath::FWidgetPropertyPath(const FStringView& InText)
{
	FString ParseError;
	if (!TryParse(InText, *this, &ParseError))
	{
		Reset();
		ensureMsgf(false, TEXT("Invalid widget property path '%s': %s"), *FString(InText), *ParseError);
	}
}

FWidgetPropertyPath::FWidgetPropertyPath(const TCHAR* InText)
	: FWidgetPropertyPath(FStringView(InText ? InText : TEXT("")))
{
}

bool FWidgetPropertyPath::TryParse(const FStringView& InText, FWidgetPropertyPath& OutPath, FString* OutError)
{
	OutPath.Reset();

	const FString Text(InText);
	if (Text.IsEmpty())
	{
		if (OutError)
		{
			*OutError = TEXT("Property path must not be empty.");
		}
		return false;
	}

	int32 Position = 0;
	while (Position < Text.Len())
	{
		const int32 SegmentStart = Position;
		while (Position < Text.Len() && Text[Position] != TCHAR('.') && Text[Position] != TCHAR('['))
		{
			++Position;
		}

		const FString PropertyToken = Text.Mid(SegmentStart, Position - SegmentStart);
		const bool bBracketSegmentFollows = Position < Text.Len() && Text[Position] == TCHAR('[');
		if (PropertyToken.IsEmpty())
		{
			if (!bBracketSegmentFollows)
			{
				if (OutError)
				{
					*OutError = FString::Printf(TEXT("Invalid empty property segment in property path '%s'."), *Text);
				}
				return false;
			}
		}
		else if (PropertyToken == TEXT("*"))
		{
			OutPath.AppendAnyProperty();
		}
		else if (IsValidPropertyToken(PropertyToken))
		{
			OutPath.AppendProperty(PropertyToken);
		}
		else
		{
			if (OutError)
			{
				*OutError = FString::Printf(TEXT("Invalid property segment '%s' in property path '%s'."), *PropertyToken, *Text);
			}
			return false;
		}

		while (Position < Text.Len() && Text[Position] == TCHAR('['))
		{
			const int32 TokenStart = Position + 1;
			const int32 TokenEnd = Text.Find(TEXT("]"), ESearchCase::CaseSensitive, ESearchDir::FromStart, TokenStart);
			if (TokenEnd == INDEX_NONE)
			{
				if (OutError)
				{
					*OutError = FString::Printf(TEXT("Unterminated bracket segment in property path '%s'."), *Text);
				}
				return false;
			}

			const FString BracketToken = Text.Mid(TokenStart, TokenEnd - TokenStart);
			if (BracketToken.IsEmpty())
			{
				if (OutError)
				{
					*OutError = FString::Printf(TEXT("Empty bracket segment in property path '%s'."), *Text);
				}
				return false;
			}

			if (BracketToken == TEXT("*"))
			{
				OutPath.AppendAnyArrayIndex();
			}
			else
			{
				int32 ArrayIndex = INDEX_NONE;
				if (!TryParseArrayIndexToken(BracketToken, ArrayIndex))
				{
					if (OutError)
					{
						*OutError = FString::Printf(TEXT("Non-numeric bracket segment '[%s]' is not supported in property path '%s'."), *BracketToken, *Text);
					}
					return false;
				}
				OutPath.AppendArrayIndex(ArrayIndex);
			}

			Position = TokenEnd + 1;
		}

		if (Position >= Text.Len())
		{
			break;
		}

		if (Text[Position] != TCHAR('.'))
		{
			if (OutError)
			{
				*OutError = FString::Printf(TEXT("Unexpected character '%c' in property path '%s'."), Text[Position], *Text);
			}
			return false;
		}

		++Position;
		if (Position >= Text.Len())
		{
			if (OutError)
			{
				*OutError = FString::Printf(TEXT("Property path '%s' must not end with '.'."), *Text);
			}
			return false;
		}
	}

	return !OutPath.IsEmpty();
}

void FWidgetPropertyPath::Reset()
{
	Elements.Reset();
	PathName = NAME_None;
	bIsPathNameDirty = true;
}

bool FWidgetPropertyPath::IsEmpty() const
{
	return Elements.IsEmpty();
}

bool FWidgetPropertyPath::HasAny() const
{
	for (const FWidgetPropertyPathElement& Element : Elements)
	{
		if (Element.bIsAny)
		{
			return true;
		}
	}

	return false;
}

const FName& FWidgetPropertyPath::GetPathName() const
{
	if (bIsPathNameDirty)
	{
		FString CanonicalString;
		for (const FWidgetPropertyPathElement& Element : Elements)
		{
			if (Element.Type == EWidgetPropertyPathElementType::Property)
			{
				if (!CanonicalString.IsEmpty())
				{
					CanonicalString += TEXT(".");
				}
				CanonicalString += Element.ToString();
			}
			else
			{
				CanonicalString += Element.ToString();
			}
		}

		PathName = CanonicalString.IsEmpty() ? NAME_None : FName(*CanonicalString);
		bIsPathNameDirty = false;
	}

	return PathName;
}

bool FWidgetPropertyPath::operator==(const FWidgetPropertyPath& Other) const
{
	return GetPathName() == Other.GetPathName();
}

bool FWidgetPropertyPath::Matches(const FWidgetPropertyPath& Candidate) const
{
	if (Elements.Num() != Candidate.Elements.Num())
	{
		return false;
	}

	for (int32 ElementIndex = 0; ElementIndex < Elements.Num(); ++ElementIndex)
	{
		if (!Elements[ElementIndex].Matches(Candidate.Elements[ElementIndex]))
		{
			return false;
		}
	}

	return true;
}

bool FWidgetPropertyPath::TryMakeRelativeTo(const FWidgetPropertyPath& BasePath, FWidgetPropertyPath& OutRelativePath) const
{
	OutRelativePath.Reset();

	const TArray<FWidgetPropertyPathElement>& BaseElements = BasePath.GetElements();
	if (BaseElements.IsEmpty() || BaseElements.Num() > Elements.Num())
	{
		return false;
	}

	for (int32 ElementIndex = 0; ElementIndex < BaseElements.Num(); ++ElementIndex)
	{
		if (!AreElementsEqual(BaseElements[ElementIndex], Elements[ElementIndex]))
		{
			return false;
		}
	}

	if (BaseElements.Num() == Elements.Num())
	{
		return true;
	}

	FString RelativePathString;
	for (int32 ElementIndex = BaseElements.Num(); ElementIndex < Elements.Num(); ++ElementIndex)
	{
		const FWidgetPropertyPathElement& Element = Elements[ElementIndex];
		const bool bIsProperty = Element.Type == EWidgetPropertyPathElementType::Property;
		if (bIsProperty && !RelativePathString.IsEmpty())
		{
			RelativePathString.AppendChar(TCHAR('.'));
		}

		RelativePathString.Append(Element.ToString());
	}

	if (RelativePathString.IsEmpty())
	{
		return true;
	}

	return FWidgetPropertyPath::TryParse(FStringView(RelativePathString), OutRelativePath);
}

FWidgetPropertyPath FWidgetPropertyPath::WithAppendedProperty(const FStringView& PropertyPathString) const
{
	FWidgetPropertyPath NewPath = *this;
	FWidgetPropertyPath AppendedPath;
	const FString PropertyPathText(PropertyPathString);
	checkf(FWidgetPropertyPath::TryParse(PropertyPathString, AppendedPath), TEXT("Invalid property path string '%s'."), *PropertyPathText);
	NewPath.Elements.Append(AppendedPath.Elements);
	NewPath.MarkPathNameDirty();
	return NewPath;
}

FWidgetPropertyPath FWidgetPropertyPath::WithAppendedAnyProperty() const
{
	FWidgetPropertyPath NewPath = *this;
	NewPath.AppendAnyProperty();
	return NewPath;
}

FWidgetPropertyPath FWidgetPropertyPath::WithAppendedArrayIndex(int32 ArrayIndex) const
{
	FWidgetPropertyPath NewPath = *this;
	NewPath.AppendArrayIndex(ArrayIndex);
	return NewPath;
}

FWidgetPropertyPath FWidgetPropertyPath::WithAppendedAnyArrayIndex() const
{
	FWidgetPropertyPath NewPath = *this;
	NewPath.AppendAnyArrayIndex();
	return NewPath;
}

void FWidgetPropertyPath::AppendProperty(const FStringView& PropertyName)
{
	Elements.Add(FWidgetPropertyPathElement::MakeProperty(PropertyName));
	MarkPathNameDirty();
}

void FWidgetPropertyPath::AppendAnyProperty()
{
	Elements.Add(FWidgetPropertyPathElement::MakeAnyProperty());
	MarkPathNameDirty();
}

void FWidgetPropertyPath::AppendArrayIndex(int32 ArrayIndex)
{
	Elements.Add(FWidgetPropertyPathElement::MakeArrayIndex(ArrayIndex));
	MarkPathNameDirty();
}

void FWidgetPropertyPath::AppendAnyArrayIndex()
{
	Elements.Add(FWidgetPropertyPathElement::MakeAnyArrayIndex());
	MarkPathNameDirty();
}

void FWidgetPropertyPath::MarkPathNameDirty()
{
	bIsPathNameDirty = true;
}

const TArray<FWidgetPropertyPathElement>& FWidgetPropertyPath::GetElements() const
{
	return Elements;
}
