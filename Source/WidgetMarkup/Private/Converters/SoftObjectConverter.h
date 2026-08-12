// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "Converter.h"

class FSoftObjectConverter : public FConverter
{
public:
	static TSharedRef<FConverter> Create();

protected:
	virtual bool Convert(const FProperty& Property, void* Data, const FStringView& String) override;
};
