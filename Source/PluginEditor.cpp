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
	 myTabbedComponent(TabbedButtonBar::Orientation::TabsAtTop)
{
	//testFFT();
	buffer = new Queue<double>();
	p.setBufferPointer(buffer);
	
	// The tabs are the main attraction of this component
	addAndMakeVisible(myTabbedComponent);
	setName("Dogmatics");

	//EffectsPanel *ep = new EffectsPanel(processor);
	//ep->Component::setBoundsInset(BorderSize(0));
	//myTabbedComponent.addTab("Effects", Colours::black, ep, true);
	
	SpectralAnalyzer *sa = new SpectralAnalyzer(buffer);
	sa->Component::setBoundsInset(BorderSize(0));
	sa->setSampleRate(p.getSampleRate());
	myTabbedComponent.addTab("Spectral Analyzer", Colours::black, sa, true);
	
	hd = new HarmonicDesigner(*this);
	hd->Component::setBoundsInset(BorderSize(0));
	myTabbedComponent.addTab("Harmonic Designer", Colours::black, hd, true);
	//myTabbedComponent.currentTabChanged(1, "Harmonic Designer");
	hd->addActionListener(this);

	Oscilloscope *osc = new Oscilloscope(*this);
	osc->Component::setBoundsInset(BorderSize(0));
	myTabbedComponent.addTab("Oscilloscope", Colours::black, osc, true);
	
	addAndMakeVisible(myKeyboard);
	myTabbedComponent.setBoundsInset(BorderSize(0));
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

void DogmaticsAudioProcessorEditor::resized()
{
	// Put a 20-pixel padding in between all elements
	const int w = getWidth(),
		h = getHeight(),
		padding = 20;
	const float keyboardScale = 0.15f;

	// Set the bounds of the tabbed elements.
	myTabbedComponent.setBounds(padding,padding,w - 2 * padding, lround(h*(1.0f - keyboardScale)) - 2 * padding);

	// Set the bounds of the keyboard and scale the keys to the width of the plugin
	myKeyboard.setBounds(padding,lround(h * (1.0f - keyboardScale)) - padding,	w - padding * 2, lround(h * keyboardScale));
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
