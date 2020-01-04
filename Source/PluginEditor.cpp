#include "Types.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Oscilloscope.h"
#include "HarmonicDesigner.h"
#include "SpectralAnalyzer.h"
#include "EffectsPanel.h"


using namespace juce;
using namespace Kyle;

//==============================================================================
// Constructor
DogmaticsAudioProcessorEditor::DogmaticsAudioProcessorEditor(DogmaticsAudioProcessor& p) : AudioProcessorEditor(p), processor(p),
	myKeyboard(p.myKeyboardState, MidiKeyboardComponent::horizontalKeyboard),
	myContainer(make_unique<Component>("Container")),
	myTabbedComponent(TabbedButtonBar::Orientation::TabsAtTop)
{	
	buffer = new Queue<double>();
	p.setBufferPointer(buffer);

	setName("Dogmatics");

	myEffectsPanel = make_unique<EffectsPanel>(processor);
	mySpectralAnalyzer = make_unique<SpectralAnalyzer>(buffer);
	myHarmonicDesigner = make_unique<HarmonicDesigner>(*this);
	myOscilloscope = make_unique<Oscilloscope>(*this);

	mySpectralAnalyzer->setSampleRate(p.getSampleRate());
	myHarmonicDesigner->addActionListener(this);
	
	if (use_tabs) {
		addAndMakeVisible(myTabbedComponent);
		myTabbedComponent.setBoundsRelative(0.f, 0.f, 1.f, 1.f - keyboardScale);

		myEffectsPanel->setBounds(getLocalBounds());
		myTabbedComponent.addTab("Effects", Colours::black, myEffectsPanel.get(), true);

		mySpectralAnalyzer->setBounds(getLocalBounds());
		myTabbedComponent.addTab("Spectral Analyzer", Colours::black, mySpectralAnalyzer.get(), true);

		myHarmonicDesigner->setBounds(getLocalBounds());
		myTabbedComponent.addTab("Harmonic Designer", Colours::black, myHarmonicDesigner.get(), true);

		myOscilloscope->setBounds(getLocalBounds());
		myTabbedComponent.addTab("Oscilloscope", Colours::black, myOscilloscope.get(), true);
	}
	else {
		addChildComponent(*myContainer);
		myContainer->setBoundsRelative(0.f, 0.f, 1.f, 1.f - keyboardScale);
		myContainer->setVisible(true);

		myContainer->addAndMakeVisible(myEffectsPanel.get());
		myEffectsPanel->setBoundsRelative(0.5f, 0.5f, 0.5f, 0.5f);

		myContainer->addAndMakeVisible(mySpectralAnalyzer.get());
		mySpectralAnalyzer->setBoundsRelative(0.5f, 0.f, 0.5f, 0.5f);

		myContainer->addAndMakeVisible(myHarmonicDesigner.get());
		myHarmonicDesigner->setBoundsRelative(0.f, 0.5f, 0.5f, 0.5f);

		myContainer->addAndMakeVisible(myOscilloscope.get());
		myOscilloscope->setBoundsRelative(0.f, 0.f, 0.5f, 0.5f);
	}
	
	addAndMakeVisible(myKeyboard);
	myKeyboard.setBoundsRelative(0.f, 1.f - keyboardScale, 1.f, keyboardScale);
	myKeyboard.setKeyWidth((float)myKeyboard.getWidth() / 75.0f);

	setResizable(true, true);
	setSize(1200, 800);
}

DogmaticsAudioProcessor& DogmaticsAudioProcessorEditor::getProcessor() const
{
	return static_cast<DogmaticsAudioProcessor&> (processor);
}

DogmaticsAudioProcessorEditor::~DogmaticsAudioProcessorEditor()
{
}

//==============================================================================
void DogmaticsAudioProcessorEditor::paint (Graphics& g)
{
	ColourGradient grad(Colours::darkgrey, 0.0f, 0.0f, Colours::black, (float)getWidth(), (float)getHeight(), false);
	g.setGradientFill(grad);
	g.fillAll();
}

void DogmaticsAudioProcessorEditor::mouseMove(const MouseEvent & theEvent) {
	repaint();
}

void DogmaticsAudioProcessorEditor::harmonicsChanged() {
	myHarmonicDesigner->notifyHarmonicsChanged(); 
}

void DogmaticsAudioProcessorEditor::resized()
{
	if(use_tabs) {
		myTabbedComponent.setBoundsRelative(0.f, 0.f, 1.f, 1.f - keyboardScale);
	}
	else {
		myContainer->setBoundsRelative(0.f, 0.f, 1.f, 1.f - keyboardScale);
		if (myEffectsPanel)
			myEffectsPanel->setBoundsRelative(0.5f, 0.5f, 0.5f, 0.5f);
		if (mySpectralAnalyzer)
			mySpectralAnalyzer->setBoundsRelative(0.5f, 0.f, 0.5f, 0.5f);
		if (myHarmonicDesigner)
			myHarmonicDesigner->setBoundsRelative(0.f, 0.5f, 0.5f, 0.5f);
		if (myOscilloscope)
			myOscilloscope->setBoundsRelative(0.f, 0.f, 0.5f, 0.5f);
	}

	myKeyboard.setBoundsRelative(0.f, 1.f-keyboardScale, 1.f, keyboardScale);
	myKeyboard.setKeyWidth((float)myKeyboard.getWidth() / 75.0f);
}

void DogmaticsAudioProcessorEditor::setSampleRate(int theSampleRate) {
//	if (spectral) {
//		spectral->setSampleRate(theSampleRate);
//	}
}

void DogmaticsAudioProcessorEditor::actionListenerCallback(const String & message) {
	string s = message.toStdString();
	int space = s.find(' ');
	int bin = stoi(s.substr(0, space));
	double mag = stod(s.substr(space + 1, s.length()));
	getProcessor().changeHarmonic(bin, mag);
}
