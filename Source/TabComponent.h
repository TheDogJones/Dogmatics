#pragma once

#ifndef _KYLE_TAB_COMPONENT_
#define _KYLE_TAB_COMPONENT_

#include "Types.h"
#include "PluginEditor.h"

using namespace std;
using namespace Kyle;
using namespace juce;

class TabComponent : public Component
{
public:
	TabComponent(DogmaticsAudioProcessorEditor &p, Queue<double> *theBuffer);
	~TabComponent();

	void resized() override;
	void paint(Graphics& g) override;

private:
	DogmaticsAudioProcessorEditor& parent;

	DogmaticsAudioProcessorEditor& getParent() const;

};

#endif //_KYLE_TAB_COMPONENT_