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
	myContainer(make_unique<Component>("Container"))
{
	setResizable(true, true);
	setSize(1200, 800);
	//testFFT();
	buffer = new Queue<double>();
	p.setBufferPointer(buffer);
	
	addChildComponent(*myContainer);
	myContainer->setBoundsRelative(0.f, 0.f, 1.f, 0.85f);
	myContainer->setVisible(true);

	// The tabs are the main attraction of this component
	//addAndMakeVisible(myTabbedComponent);
	setName("Dogmatics");

	myEffectsPanel = make_unique<EffectsPanel>(processor);
	myContainer->addAndMakeVisible(*myEffectsPanel);
	myEffectsPanel->setBoundsRelative(0.5f, 0.5f, 0.5f, 0.5f);
	//myTabbedComponent.addTab("Effects", Colours::black, ep, true);
	
	mySpectralAnalyzer = make_unique<SpectralAnalyzer>(buffer);
	myContainer->addAndMakeVisible(*mySpectralAnalyzer);
	mySpectralAnalyzer->setBoundsRelative(0.5f, 0.f, 0.5f, 0.5f);
	mySpectralAnalyzer->setSampleRate(p.getSampleRate());
	//myTabbedComponent.addTab("Spectral Analyzer", Colours::black, sa, true);
	
	myHarmonicDesigner = make_unique<HarmonicDesigner>(*this);
	myContainer->addAndMakeVisible(*myHarmonicDesigner);
	myHarmonicDesigner->setBoundsRelative(0.f, 0.5f, 0.5f, 0.5f);
	//myTabbedComponent.addTab("Harmonic Designer", Colours::black, myHarmonicDesigner, true);
	myHarmonicDesigner->addActionListener(this);

	myOscilloscope = make_unique<Oscilloscope>(*this);
	myContainer->addAndMakeVisible(*myOscilloscope);
	myOscilloscope->setBoundsRelative(0.f, 0.f, 0.5f, 0.5f);
	//myTabbedComponent.addTab("Oscilloscope", Colours::black, osc, true);
	
	//myKeyboard.getOctaveForMiddleC
	addAndMakeVisible(myKeyboard);
	myKeyboard.setBoundsRelative(0.f, 0.85f, 1.f, 0.15f);
	myKeyboard.setKeyWidth((float)myKeyboard.getWidth() / 75.0f);
	//myTabbedComponent.setBoundsInset(BorderSize(0));
	
	//processor.editorCallBack(this);
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
	auto r = myContainer->getBoundsInParent();
	myContainer->setBoundsRelative(0.f, 0.f, 1.f, 1.f - keyboardScale);
	if (myEffectsPanel)
		myEffectsPanel->setBoundsRelative(0.5f, 0.5f, 0.5f, 0.5f);
	if (mySpectralAnalyzer)
		mySpectralAnalyzer->setBoundsRelative(0.5f, 0.f, 0.5f, 0.5f);
	if (myHarmonicDesigner)
		myHarmonicDesigner->setBoundsRelative(0.f, 0.5f, 0.5f, 0.5f); 
	if (myOscilloscope)
		myOscilloscope->setBoundsRelative(0.f, 0.f, 0.5f, 0.5f);

	myKeyboard.setBoundsRelative(0.f, 1.f-keyboardScale, 1.f, keyboardScale);
	
	
	const int w = getWidth(), h = getHeight();	
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
