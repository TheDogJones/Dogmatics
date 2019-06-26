#pragma once

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "Types.h"

#include "PluginEditor.h"
#include "HarmonicSynth.h"

using namespace std;
using namespace Kyle;
using namespace juce;

//==============================================================================
/**
*/
class DogmaticsAudioProcessor : public AudioProcessor
{
public:
	//==============================================================================
	DogmaticsAudioProcessor();
	~DogmaticsAudioProcessor();

	// Important stuff
	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;

	void releaseResources() override;
	
	void setBufferPointer(Queue<double>::Ptr buffer);
	
	void changeHarmonic(int theBin, double theMagnitude);

	void setHarmonics(Wave theWave);

	Complex *getHarmonics() { return myHarmonics; }
	
	void initializeSynth();
	
	void processBlock(AudioBuffer<float>&, MidiBuffer&) override;
	
	Queue<double>::Ptr sampleBuffer;

	// Housekeeping crap
	//==============================================================================
	AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override { return true; } // myEditor != nullptr;
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override { return true; }
	bool isMidiEffect() const override { return false; }
	const String getName() const { return JucePlugin_Name; }
	bool acceptsMidi() const override { return true; }
	bool producesMidi() const override { return false; }
	double getTailLengthSeconds() const override { return 0.1; }
	int getNumPrograms() override { return 1; }
	int getCurrentProgram() override { return 0; }
	void setCurrentProgram(int /*index*/) {}
	const String getProgramName(int /*index*/) override { return String(); }
	void changeProgramName(int /*index*/, const String& /*newName*/) override {}
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	MidiKeyboardState myKeyboardState;
	AudioProcessorValueTreeState state;
	
private:
	bool initializingSynth;

	HeapBlock<Complex> myHarmonics;
	Synthesiser mySynthesizer;
	AudioParameterFloat *masterGain;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DogmaticsAudioProcessor)
};

#endif //PLUGINPROCESSOR_H_INCLUDED