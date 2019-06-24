#pragma once

#include "Types.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "TabComponent.h"
#include "Oscilloscope.h"
#include "HarmonicDesigner.h"

using namespace std;
using namespace Kyle;
using namespace juce;

TabComponent::TabComponent(DogmaticsAudioProcessorEditor &p, Queue<double> *theBuffer) : Component("Main"), parent(p),
	hd(new HarmonicDesigner(*this)), osc(new Oscilloscope(theBuffer, *this)), 
{
	addAndMakeVisible(myTabbedComponent);
	setName("Dogmatics");
	//SpectralAnalyzer *sa = new SpectralAnalyzer(p.sampleBuffer, &p);
	//hd->addActionListener(p);
	hd->Component::setBoundsInset(BorderSize(0));
	myTabbedComponent->addTab("Harmonic Designer", Colours::black, hd, true);
	osc->Component::setBoundsInset(BorderSize(0));
	myTabbedComponent->addTab("Oscilloscope", Colours::black, osc, true);
	//myTabbedComponent.addTab("Spectral Analyzer", Colours::black, sa, true);
	//myTabbedComponent.addTab("Button", Colours::black, mcc, true);
	setBoundsInset(BorderSize(0));
	
}

void TabComponent::paint(Graphics& g) {
}

void TabComponent::resized() {
}


TabComponent::~TabComponent()
{
	
}

//==============================================================================
