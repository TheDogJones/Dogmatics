#pragma once
#ifndef __KYLE_EFFECTSPANEL__
#define __KYLE_EFFECTSPANEL__

#include "Types.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace std;
using namespace Kyle;
using namespace juce;

//==========================================================================================
// EffectsPanel.h: Main panel for displaying effects controls
//==========================================================================================
class EffectsPanel : public Component {
public:
	EffectsPanel(DogmaticsAudioProcessor &p);
	~EffectsPanel();

	//void clear();
	void resized() override;
	void paint(Graphics& g) override;

	DogmaticsAudioProcessor &getProcessor() const {
		return static_cast<DogmaticsAudioProcessor&> (processor);
	}

private:
	DogmaticsAudioProcessor &processor;
	Slider gain;
	TextButton sine;
	TextButton saw;
	TextButton square;
	TextButton triangle;
	AudioProcessorValueTreeState::SliderAttachment gainAttachment;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsPanel)
};
#endif