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
	container(new Component("Container"))
{
	//testFFT();
	buffer = new Queue<double>();
	p.setBufferPointer(buffer);
	
	container->setBoundsRelative(0.f, 0.f, 1.f, 0.85f);
	addAndMakeVisible(container);

	// The tabs are the main attraction of this component
	//addAndMakeVisible(myTabbedComponent);
	setName("Dogmatics");

	EffectsPanel *ep = new EffectsPanel(processor);
	ep->setBoundsRelative(0.5f, 0.5f, 0.5f, 0.5f);
	//myTabbedComponent.addTab("Effects", Colours::black, ep, true);
	container->addAndMakeVisible(ep);
	
	SpectralAnalyzer *sa = new SpectralAnalyzer(buffer);
	sa->setBoundsRelative(0.5f, 0.f, 0.5f, 0.5f);
	sa->setSampleRate(p.getSampleRate());
	//myTabbedComponent.addTab("Spectral Analyzer", Colours::black, sa, true);
	container->addAndMakeVisible(sa);
	
	hd = new HarmonicDesigner(*this);
	hd->setBoundsRelative(0.f, 0.5f, 0.5f, 0.5f);
	//myTabbedComponent.addTab("Harmonic Designer", Colours::black, hd, true);
	container->addAndMakeVisible(hd);
	hd->addActionListener(this);

	Oscilloscope *osc = new Oscilloscope(*this);
	osc->setBoundsRelative(0.f, 0.f, 0.5f, 0.5f);
	//myTabbedComponent.addTab("Oscilloscope", Colours::black, osc, true);
	container->addAndMakeVisible(osc);
	
	//myKeyboard.getOctaveForMiddleC
	myKeyboard.setBoundsRelative(0.f, 0.85f, 1.f, 0.15f);
	addAndMakeVisible(myKeyboard);
	//myTabbedComponent.setBoundsInset(BorderSize(0));
	setResizable(true, true);
	setSize(1200, 800);
	
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
	hd->notifyHarmonicsChanged(); 
}

void DogmaticsAudioProcessorEditor::resized()
{
	/*
	// Put a 20-pixel padding in between all elements
	const int w = getWidth(),
		h = getHeight(),
		padding = 20;
	const float keyboardScale = 0.15f;

	// Set the bounds of the tabbed elements.
	//container->setBounds(padding,padding,w - 2 * padding, lround(h*(1.0f - keyboardScale)) - 2 * padding);

	// Set the bounds of the keyboard and scale the keys to the width of the plugin
	myKeyboard.setBounds(padding,lround(h * (1.0f - keyboardScale)) - padding,	w - padding * 2, lround(h * keyboardScale));
	myKeyboard.setKeyWidth((float)myKeyboard.getWidth() / 75.0f);*/
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
