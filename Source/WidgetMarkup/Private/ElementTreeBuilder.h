// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "ElementNode.h"
#include "FastXml.h"

class FWidgetMarkupModule;
class UWidgetTree;

class FElementTreeBuilder : public IFastXmlCallback, public FGCObject
{
public:
	FElementTreeBuilder(UObject* InOuter);
	virtual ~FElementTreeBuilder() override = default;

	//~Begin IFastXmlCallback interface
	virtual bool ProcessXmlDeclaration(const TCHAR* ElementData, int32 XmlFileLineNumber) override;
	virtual bool ProcessElement(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber) override;
	virtual bool ProcessAttribute(const TCHAR* AttributeName, const TCHAR* AttributeValue) override;
	virtual bool ProcessClose(const TCHAR* Element) override;
	virtual bool ProcessComment(const TCHAR* Comment) override;
	//~End IFastXmlCallback interface

	//~Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~End FGCObject interface

	TSharedPtr<FElementNode> GetRootElementNode();
	TSharedPtr<FElementNode> GetCurrentElementNode() const;

	/** First error message collected while parsing, for status reporting. */
	const FText& GetFirstErrorText() const { return FirstErrorText; }

protected:
	void CaptureFirstError(const FElementNode::FResult& Result);

	TObjectPtr<UObject> Outer;
	FElementNode::FContext Context;
	TSharedPtr<FElementNode> RootElementNode;
	FWidgetMarkupModule* WidgetMarkupModule = nullptr;
	FText FirstErrorText;
};
