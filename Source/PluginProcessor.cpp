#pragma once

#include "Types.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "HarmonicSynth.h"
#include "Components/Parameter.h"

using namespace std;
using namespace Kyle;
using namespace juce;

//==============================================================================
// Constructor
DogmaticsAudioProcessor::DogmaticsAudioProcessor() : state(*this, nullptr, "state", {
	std::make_unique<AudioParameterFloat>("masterGain", "Gain", NormalisableRange<float>(0.0f, 1.0f), 0.9f) 
	}),
	AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo(), true)
									.withOutput("Output", AudioChannelSet::stereo(), true))
{
	// Initialize harmonics. Default is a saw wave.
	myHarmonics. allocate(WAVEFORM_SIZE, true);
	Sine(myHarmonics, WAVEFORM_SIZE);
	initializeSynth();
}

DogmaticsAudioProcessor::~DogmaticsAudioProcessor()
{
	myKeyboardState.reset();
	mySynthesizer.clearSounds();
	mySynthesizer.clearVoices();
}

//==============================================================================
// Initialize synth. Call this function whenever harmonics are modified.
void DogmaticsAudioProcessor::initializeSynth() {
	const int maxVoices = 16;

	mySynthesizer.clearVoices();
	mySynthesizer.clearSounds();

	for (int i = 0; i < maxVoices; i++) {
		mySynthesizer.addVoice(new HarmonicWaveformVoice(myHarmonics));
	}
	mySynthesizer.addSound(new HarmonicWaveformSound());
}

//==============================================================================

void DogmaticsAudioProcessor::changeHarmonic(int theBin, double theReal, double theImag) {
	myHarmonics[theBin] = Complex(theReal, theImag);
	
	for (int i = 0; i < mySynthesizer.getNumVoices(); i++) {
		((HarmonicWaveformVoice*)mySynthesizer.getVoice(i))->updateHarmonic(theBin, theReal, theImag);
	}
}

void DogmaticsAudioProcessor::setHarmonics(Wave theWave) {
	theWave(myHarmonics, WAVEFORM_SIZE);

	for (int i = 0; i < mySynthesizer.getNumVoices(); i++) {
		((HarmonicWaveformVoice*)mySynthesizer.getVoice(i))->setHarmonics(myHarmonics);
	}
	((DogmaticsAudioProcessorEditor*)getActiveEditor())->harmonicsChanged();
}

void DogmaticsAudioProcessor::setBufferPointer(Queue<double>::Ptr buffer) {
	sampleBuffer = buffer;
}

void DogmaticsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
	ignoreUnused(samplesPerBlock);
	mySynthesizer.setCurrentPlaybackSampleRate(sampleRate);
	myKeyboardState.reset();
	reset();

	//DogmaticsAudioProcessorEditor* editor = (DogmaticsAudioProcessorEditor*)getActiveEditor();
	//if (editor) {
	//	editor->setSampleRate((int)sampleRate);
	//}
}

void DogmaticsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
	myKeyboardState.reset();
}

void DogmaticsAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	buffer.clear();
	const int totalNumOutputChannels = getTotalNumOutputChannels();
	const int numSamples = buffer.getNumSamples();


	// Now pass any incoming midi messages to our keyboard state object, and let it
	// add messages to the buffer if the user is clicking on the on-screen keys
	myKeyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);

	// and now get our synth to process these midi events and generate its output.
	mySynthesizer.renderNextBlock(buffer, midiMessages, 0, numSamples);

	buffer.applyGain(*state.getRawParameterValue("masterGain"));

	//=======================================================================================

	if (sampleBuffer != nullptr) {
		for (int i = 0; i < numSamples; i++) {
			sampleBuffer->Push(buffer.getSample(0, i));
		}
	}

	//=======================================================================================

	midiMessages.clear();
}

//==============================================================================
void DogmaticsAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DogmaticsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

AudioProcessorEditor* DogmaticsAudioProcessor::createEditor()
{
	return new DogmaticsAudioProcessorEditor(*this); 
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DogmaticsAudioProcessor();
}
