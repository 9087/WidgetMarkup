// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ElementNode.h"

class FProperty;
struct FPropertyBuffer;

/**
 * Shared helpers for assembling element values into typed property buffers.
 * Used by both the Blueprint Variable pipeline and the Style Setter pipeline.
 */
namespace FPropertyValueAssembler
{
	/**
	 * Copies a single child element's value (Basic/Struct/Object) into the
	 * target buffer, converting via the converter registry as needed.
	 */
	FElementNode::FResult CopyChildIntoBuffer(FProperty* Property, FPropertyBuffer& TargetBuffer, const TSharedRef<FElementNode>& Child);

	/**
	 * Appends one child element into a container value (Array/Set/Map) held by
	 * the target buffer. Array/Set accept Basic/Struct/Object children; Map
	 * requires FWidgetMarkupKeyValuePair (Pair) children.
	 */
	FElementNode::FResult AppendChildToContainer(FProperty* ContainerProperty, FPropertyBuffer& TargetBuffer, const TSharedRef<FElementNode>& Child);
}
