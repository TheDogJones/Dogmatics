#include "Types.h"
#include "EffectsPanel.h"

using namespace juce;
using namespace Kyle;


EffectsPanel::EffectsPanel(DogmaticsAudioProcessor &p) : processor(p),
	gainAttachment(p.state, "masterGain", gain)
{
	gain.setSliderStyle(Slider::LinearBarVertical);
	gain.setRange(0.0, 1.0, 0.01);
	gain.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
	gain.setPopupDisplayEnabled(true, false, this);
	gain.setTextValueSuffix(" Volume");
	gain.setValue(1.0);

	gain.setBounds(677, 252, 40, 148);

	sine.setButtonText("Sine");
	//sine.

	addAndMakeVisible(gain);
}

EffectsPanel::~EffectsPanel() {
}

void EffectsPanelclear() {}
void EffectsPanel::resized() { 
	gain.setBounds(40, 30, 20, getHeight() - 60); 
}

void EffectsPanel::paint(Graphics& g) {
	// fill the whole window white
	g.fillAll(Colours::black);
	// set the current drawing colour to black
	g.setColour(Colours::white);
	// set the font size and draw text to the screen
	g.setFont(15.0f);
	g.drawFittedText("Gain", 0, 0, getWidth(), 30, Justification::centred, 1);
}

void EffectsPanel::buttonClicked(Button *) {
	//(DogmaticsAudioProcessorEditor*)(processor.getActiveEditor())->
}
