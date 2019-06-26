#pragma once

#ifndef _KYLE_HARMONIC_SYNTH_
#define _KYLE_HARMONIC_SYNTH_

#include "Types.h"

using namespace std;
using namespace Kyle;
using namespace juce;

//=======================================================================================
// HarmonicSynth.h: Classes for converting harmonic arrays into actual signal data to be
// sent to the sound card. Must run fast!
//=======================================================================================
class HarmonicWaveformSound : public SynthesiserSound {
public:
	HarmonicWaveformSound() {}
	~HarmonicWaveformSound() {}

	bool appliesToNote(int /*midiNoteNumber*/) override {
		return true;
	}
	bool appliesToChannel(int /*midiChannel*/) override {
		return true;
	}
};

class HarmonicWaveformVoice : public SynthesiserVoice {
public:
	// Constructor
	HarmonicWaveformVoice(Complex *theHarmonics) : tailOff(0.0) {

		myHarmonics.allocate(WAVEFORM_SIZE, true);
		mySamples.allocate(WAVEFORM_SIZE, false);

		// Collect harmonics to be transformed into samples
		for (int i = 0; i < WAVEFORM_SIZE; i++) {
			myHarmonics[i] = Complex(theHarmonics[i]);
		}

		generateWaveform();

		// Run the FFT on our harmonics to get waveform
		waveScale = WAVEFORM_SIZE / (2 * pi);
	}

	~HarmonicWaveformVoice() {
		mySamples.free();
	}

	void generateWaveform() {
		for (int i = 0; i < WAVEFORM_SIZE; i++) {
			mySamples[i] = Complex(myHarmonics[i]);
		}
		Kyle::FFT::DoItBackwards(mySamples, WAVEFORM_SIZE);
	}

	void updateHarmonic(int bin, double mag) {
		myHarmonics[bin] = Complex(0, mag);
		generateWaveform();
	}

	void setHarmonics(Complex *theHarmonics) {
		for (int i = 0; i < WAVEFORM_SIZE; i++) {
			myHarmonics[i] = Complex(theHarmonics[i]);
		}
		generateWaveform();
	}

	bool canPlaySound(SynthesiserSound* sound) override {
		return dynamic_cast<HarmonicWaveformSound*> (sound) != nullptr;
	}

	void startNote(int midiNoteNumber, float velocity,
				   SynthesiserSound* /*sound*/,
				   int /*currentPitchWheelPosition*/) override {
		myTheta = 0.0;
		level = velocity * 0.2;
		tailOff = 0.0;
		
		// Get the fundamental double of our midi note
		double freq = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
		//DBG(log2(freq));

		// Assuming our harmonic #1 is the fundamental, find the harmonic that
		// passes the Nyquist double, and cutoff everything above that.
		// This is how we prevent aliasing.
		int cutoff_bin = floor((getSampleRate() / 2.0f / freq));

		// myDelta is the amount the phase of the waveform will shift with each
		// sample. Will be larger on higher notes (higher freq.)
		myDelta = 2 * pi / cutoff_bin;
	}

	void stopNote(float /*velocity*/, bool allowTailOff) override {
		if(allowTailOff) {
			// start a tail-off by setting this flag. The render callback will pick up on
			// this and do a fade out, calling clearCurrentNote() when it's finished.
			if(tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
							   // stopNote method could be called more than once.
				tailOff = 1.0;
		}
		else {
			// we're being told to stop playing immediately, so reset everything..
			clearCurrentNote();
			myDelta = 0.0;
		}
	}

	void pitchWheelMoved(int /*newValue*/) override {
		// do this later
	}

	void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override {
		// do this later
	}

	void renderNextBlock(AudioBuffer<double>& outputBuffer, int startSample, int numSamples) override {
		render<double>(outputBuffer, startSample, numSamples);
	}

	void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
		render<float>(outputBuffer, startSample, numSamples);
	}

	template <typename FT>
	void render(AudioBuffer<FT>& outputBuffer, int startSample, int numSamples) {
		int theStartSample = startSample;
		int theNumSamples = numSamples;
		if(myDelta != 0.0) {
			// If the note has been released, slowly tail off.
			if(tailOff > 0) {
				while(--theNumSamples >= 0) {
					// Use cubic interpolation to get a good estimate of the correct amplitude at this position.
					// Note: this algorithm is imperfect and potentially noticeable for sound data. Will need to migrate
					// this later to the "mathematically perfect" interpolation algorithm for better sound quality.
					const FT currentSample = CubicInterpolate(mySamples, WAVEFORM_SIZE, myTheta*waveScale) * tailOff * level;

					for (int i = outputBuffer.getNumChannels(); --i >= 0;) {
						outputBuffer.addSample(i, theStartSample, (FT)currentSample);
					}
					
					// Theta is current position, add delta to theta, and mod by 2*pi.
					// (mod by 2*pi is technically unnecessary, but makes math easier to read
					// for debugging and stuff
					myTheta = fmod(myTheta + myDelta, 2.0f*pi);
					theStartSample +=OVERSAMPLE_FACTOR;

					// Slightly diminish amplitude until we reach a minimum threshold, then turn off note.
					tailOff *= 0.999;

					if(tailOff <= 0.005) {
						// tells the synth that this voice has stopped
						clearCurrentNote();
						myDelta = 0.0;
						break;
					}
				}
			}
			// Note is still being held down.
			else {
				while(--theNumSamples >= 0) {
					// Same interpolation as above.
					const FT currentSample = CubicInterpolate(mySamples, WAVEFORM_SIZE, myTheta*waveScale) *level;
					for(int i = outputBuffer.getNumChannels(); --i >= 0;)
						outputBuffer.addSample(i, theStartSample, (FT)currentSample);

					//oversamples[theStartSample] = (FT)currentSample;
					myTheta = fmod(myTheta + myDelta, 2.0f * pi);
					theStartSample +=OVERSAMPLE_FACTOR;
				}
			}

			// LPF
			//dsp::AudioBlock<FT> filteredSamples = new dsp::AudioBlock<FT>((SampleType)oversamples.getData(), outputBuffer.getNumChannels(), 0, numSamples*OVERSAMPLE_FACTOR);
			//dsp::ProcessorDuplicator<dsp::IIR::Filter<FT>, dsp::IIR::Coefficients<FT>> iir;
			//iir.state = dsp::IIR::Coefficients<FT>::makeLowPass(getSampleRate()*OVERSAMPLE_FACTOR, getSampleRate() / 2);
			//dsp::ProcessSpec spec{ getSampleRate()*OVERSAMPLE_FACTOR, numSamples*OVERSAMPLE_FACTOR, outputBuffer.getNumChannels() };
			//iir.prepare(spec);

			//dsp::ProcessContextReplacing<FT> context = dsp::ProcessContextReplacing<FT>::ProcessContextReplacing(filteredSamples);
			//iir.process(context);

			// Downsample and insert into buffer
			//for (int i = 0; i < numSamples; i++) {
			//	for (int j = outputBuffer.getNumChannels(); --j >= 0;) {
			//		outputBuffer.addSample(j, startSample, oversamples[i*OVERSAMPLE_FACTOR]);
			//	}
			//	startSample++;
			//}
		}
		
		// Upsample
		//dsp::AudioBlock<FT> filteredSamples = dsp::AudioBlock<FT>(outputBuffer);
		//dsp::Oversampling<FT> o = dsp::Oversampling<FT>(outputBuffer.getNumChannels(), OVERSAMPLE_FACTOR, dsp::Oversampling<FT>::filterHalfBandPolyphaseIIR);
		//o.reset();
		//o.initProcessing(WAVEFORM_SIZE);
		//dsp::AudioBlock<FT> nfs = o.processSamplesUp(filteredSamples);

		// Low pass filter
		/*dsp::ProcessorDuplicator<dsp::IIR::Filter<FT>, dsp::IIR::Coefficients<FT>> iir;
		iir.state = dsp::IIR::Coefficients<FT>::makeLowPass(getSampleRate()*OVERSAMPLE_FACTOR, getSampleRate()/4);
		dsp::ProcessSpec spec{ getSampleRate()*OVERSAMPLE_FACTOR, numSamples*OVERSAMPLE_FACTOR, outputBuffer.getNumChannels() };
		iir.prepare(spec);

		dsp::ProcessContextReplacing<FT> context = dsp::ProcessContextReplacing<FT>::ProcessContextReplacing(filteredSamples);
		iir.process(context);*/

		// Downsample
		//o.processSamplesDown(nfs);
	}

	// I don't think this will be necessary, but I don't remember why I put it here.
	Complex *getSamples() {
		return mySamples;
	}
protected:
	double level, tailOff, waveScale;
	HeapBlock<Complex> mySamples, myHarmonics;
	double myTheta, myDelta;
};
#endif // !_KYLE_HARMONIC_SYNTH_