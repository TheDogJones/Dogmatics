#pragma once
#ifndef __KYLE_SPECTRALANALYZER__
#define __KYLE_SPECTRALANALYZER__

#include "Types.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace std;
using namespace Kyle;
using namespace juce;

//===========================================================================================
// SpectralAnalyzer.h: A component for displaying the spectrum of the currently played sound.
//===========================================================================================
class SpectralAnalyzer : public OpenGLAppComponent, public Timer {
public:
	//==============================================================================
	SpectralAnalyzer(Queue<double>::Ptr theBuffer);
	~SpectralAnalyzer();

	void initialise() override;
	void clear();
	void shutdown() override;
	void resized() override;
	void paint(Graphics& g) override;
	void render() override;

	void setSampleRate(int theint);
	void timerCallback() override;
	void runFFT();

	enum {
		fftOrder = 12,             // [1]
		fftSize = 1 << fftOrder,   // [2]
		scopeSize = fftSize >> 1   // [3]
	};

private:
	//==============================================================================
	// X goes from 4 to 14.5 to represent the log2 of our
	// approximate frequency range of 16 to 23,000
	const GLfloat xMin = 3;
	const GLfloat xMax = 14.5;
	const GLfloat yMin = 0;
	const GLfloat yMax = 40;

	int mySampleRate;
	double binWidth;
	Queue<double>::Ptr myBufferPointer;

	int bufferReadPos;
	int sampleIndex;
	bool readyToRender;
	HeapBlock<Complex> mySamples;
	HeapBlock<Complex> fftData;
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectralAnalyzer);
};

#endif //__KYLE_SpectralAnalyzer__