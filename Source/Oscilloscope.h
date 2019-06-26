#pragma once
#ifndef __KYLE_OSCILLOSCOPE__
#define __KYLE_OSCILLOSCOPE__

#include "Types.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace std;
using namespace Kyle;
using namespace juce;

//==========================================================================================
// Oscilloscope.h: A component for displaying the current waveform being played.
//==========================================================================================
class Oscilloscope : public OpenGLAppComponent {
public:
	//==============================================================================
	Oscilloscope(DogmaticsAudioProcessorEditor &p);
	~Oscilloscope();

	void initialise() override;
	void clear();
	void shutdown() override;
	void resized() override;
	void paint(Graphics& g) override;
	void render() override;
	
	DogmaticsAudioProcessorEditor &getParent() const {
		return static_cast<DogmaticsAudioProcessorEditor&> (parent);
	}
private:
	//==============================================================================
	DogmaticsAudioProcessorEditor &parent;
	const GLfloat yMax = 1.3f;
	Queue<double>::Ptr myBuffer;
	int bufferReadPos;
	GLuint list;
	//==============================================================================
	JUCE_HEAVYWEIGHT_LEAK_DETECTOR(Oscilloscope)
	//JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Oscilloscope);
};

#endif //__KYLE_OSCILLOSCOPE__