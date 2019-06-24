#pragma once

#ifndef __KYLE_PARAMETER__
#define __KYLE_PARAMETER__

#include "../Types.h"

using namespace std;
using namespace Kyle;
using namespace juce;

class Parameter :
	public AudioProcessorParameter
{
public:
	Parameter();
	~Parameter();

	// Inherited via AudioProcessorParameter
	virtual float getValue() const override { return value; }
	virtual void setValue(float newValue) override { value = newValue; }
	virtual float getDefaultValue() const override { return defaultValue; }
	virtual String getName(int maximumStringLength) const override { return name; }
	virtual String getLabel() const override { return label; }
	virtual float getValueForText(const String & text) const override;

private:
	float value;
	float defaultValue;
	String name;
	String label;
};

#endif