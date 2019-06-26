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

	gain.setBoundsRelative(0.01f,0.1f,0.05f,0.5f);

	sine.setButtonText("Sine");
	sine.setBoundsRelative(0.3f, 0.1f, 0.2f, 0.1f);
	sine.onClick = [&]() { processor.setHarmonics(Sine); };

	saw.setButtonText("Saw");
	saw.setBoundsRelative(0.3f, 0.3f, 0.2f, 0.1f);
	saw.onClick = [&]() { processor.setHarmonics(Saw); };

	square.setButtonText("Square");
	square.setBoundsRelative(0.3f, 0.5f, 0.2f, 0.1f);
	square.onClick = [&]() { processor.setHarmonics(Square); };

	triangle.setButtonText("Triangle");
	triangle.setBoundsRelative(0.3f, 0.7f, 0.2f, 0.1f);
	triangle.onClick = [&]() { processor.setHarmonics(Triangle); };

	addAndMakeVisible(gain);
	addAndMakeVisible(sine);
	addAndMakeVisible(saw);
	addAndMakeVisible(square);
	addAndMakeVisible(triangle);
}

EffectsPanel::~EffectsPanel() {
}

void EffectsPanelclear() {}
void EffectsPanel::resized() { 
}

void EffectsPanel::paint(Graphics& g) {
	// fill the whole window white
	g.fillAll(Colours::black);
	// set the current drawing colour to black
	g.setColour(Colours::white);
	// set the font size and draw text to the screen
	g.setFont(15.0f);
	g.drawFittedText("Gain", 5, 0, getWidth(), 30, Justification::left, 1);
}
