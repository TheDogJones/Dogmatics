#pragma once

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "Types.h"
#include "PluginProcessor.h"

using namespace std;
using namespace juce;
using namespace Kyle;

//==============================================================================
/**
*/
class DogmaticsAudioProcessorEditor  : public AudioProcessorEditor, public ReferenceCountedObject, public ActionListener
{
public:
	using Ptr = ReferenceCountedObjectPtr<DogmaticsAudioProcessorEditor>;

    DogmaticsAudioProcessorEditor (DogmaticsAudioProcessor &p);
    ~DogmaticsAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;


	DogmaticsAudioProcessor& getProcessor() const;
	void PushSample(double theAmplitude);
	void setSampleRate(int theSampleRate);
	
	void mouseMove(const MouseEvent & theEvent);
	Queue<double>::Ptr getBuffer() { return buffer; }

	void harmonicsChanged();
		
private:
	const float keyboardScale = 0.15f;
	
	MidiKeyboardComponent myKeyboard;
	//TabbedComponent myTabbedComponent;
	unique_ptr<Component> myContainer;
	unique_ptr<Oscilloscope> myOscilloscope;
	unique_ptr<SpectralAnalyzer> mySpectralAnalyzer;
	unique_ptr<HarmonicDesigner> myHarmonicDesigner;
	unique_ptr<EffectsPanel> myEffectsPanel;

	Queue<double>::Ptr buffer;

	//HeapBlock<Complex> globalHarmonics;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DogmaticsAudioProcessor& processor;
	void actionListenerCallback(const String & message) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DogmaticsAudioProcessorEditor)
};

#endif //PLUGINEDITOR_H_INCLUDED