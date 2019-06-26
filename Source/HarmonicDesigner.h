#pragma once

#ifndef __KYLE_HARMONICDESIGNER__
#define __KYLE_HARMONICDESIGNER__

#include "Types.h"
#include "PluginEditor.h"

using namespace std;
using namespace Kyle;
using namespace juce;

//==========================================================================================
// HarmonicDesigner.h: A component for creating waveforms by adjusting individual harmonics.
//==========================================================================================

class HarmonicDesigner : public OpenGLAppComponent, public ActionBroadcaster {
public:
	//==============================================================================
	HarmonicDesigner(DogmaticsAudioProcessorEditor &p);
	~HarmonicDesigner();
	void calculateWave();

	GLfloat GlobalToWorldX(GLfloat theXClick);
	int GlobalToHarmonicX(GLfloat theXClick);
	GLfloat GlobalToMagnitudeY(GLfloat theYClick);
	GLfloat MagnitudeToWorldY(GLfloat mag, int bin);
	GLfloat NormalizeGLY(GLfloat location, GLfloat min, GLfloat max);
	GLfloat NormalizePixelY(GLfloat location, GLfloat min, GLfloat max);

	void paint(Graphics& g) override;
	void render() override;
	void resized() override;

	void mouseUp(const MouseEvent &theEvent) override;
	void mouseDrag(const MouseEvent &theEvent) override;

	void mouseWheelMove(const MouseEvent &theEvent, const MouseWheelDetails &theWheelDetails) override;

	DogmaticsAudioProcessorEditor &getParent() const {
		return static_cast<DogmaticsAudioProcessorEditor&> (parent);
	}

	void notifyHarmonicsChanged() { harmonicsChanged = true; }
	
private:
	//==============================================================================
	DogmaticsAudioProcessorEditor &parent;
	HeapBlock<Complex> mySamples;
	Complex *myHarmonics;
	const GLfloat xMin = 0.0f;
	const GLfloat xMax = (GLfloat)WAVEFORM_SIZE;
	const GLfloat yMin = -1.0f;
	const GLfloat yMax = 1.0f;
	GLfloat waveMax, waveMin;
	GLuint list;
	
	bool calculatingWave = false;
	bool harmonicsChanged = true;
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonicDesigner);

	// Inherited via OpenGLAppComponent
	void initialise() override;
	void shutdown() override;
};

#endif //__KYLE_HarmonicDesigner__