#include "Types.h"
#include "HarmonicDesigner.h"

using namespace juce;
using namespace Kyle;

//=======================================================================================
// Constructor
HarmonicDesigner::HarmonicDesigner(DogmaticsAudioProcessorEditor &p) : parent(p) {
	//myHarmonics.allocate(WAVEFORM_SIZE, true);
	myHarmonics = parent.getProcessor().getHarmonics();
	calculateWave();
}

HarmonicDesigner::~HarmonicDesigner() {
	mySamples.free();
	openGLContext.detach();
	shutdownOpenGL();
}
//=======================================================================================

void HarmonicDesigner::resized() {
	//repaint();
}

// repaint. write the text in the top left
void HarmonicDesigner::paint(Graphics & g) {
	/*g.setColour(Colours::white);
	g.setFont(15);
	g.drawText("Harmonic Designer", 25, 20, 320, 30, Justification::left);
	g.drawLine(20, 20, 220, 20);
	g.drawLine(20, 50, 220, 50);*/
}

// Draw the waveform in the background.
void HarmonicDesigner::calculateWave() {
	calculatingWave = true;
	mySamples.free();
	mySamples.allocate(WAVEFORM_SIZE, false);
	for (int i = 0; i < WAVEFORM_SIZE; i++) {
		mySamples[i] = Complex(myHarmonics[i]);
	}

	Kyle::FFT::DoItBackwards(mySamples, WAVEFORM_SIZE);
	
	// Recenter the screen around the new waveform
	waveMax = FLT_MIN_10_EXP;
	waveMin = FLT_MAX;

	for (int i = 1; i < WAVEFORM_SIZE; i++) {
		if (mySamples[i].real() < waveMin) { waveMin = mySamples[i].real(); }
		if (mySamples[i].real() > waveMax) { waveMax = mySamples[i].real(); }
	}

	calculatingWave = false;
}

//=======================================================================================
// Conversion functions for converting x and y positions relative to screen to
// OpenGL coordinates
GLfloat HarmonicDesigner::GlobalToWorldX(GLfloat theXClick) {
	return theXClick * (xMax / getWidth());
}

int HarmonicDesigner::GlobalToHarmonicX(GLfloat theXClick) {
	return clamp((int)floorl(GlobalToWorldX(theXClick) / 4),1,WAVEFORM_SIZE >> 2);
}

GLfloat HarmonicDesigner::GlobalToMagnitudeY(GLfloat theYClick) {
	if (theYClick > getHeight()) { return 0; }
	if (theYClick < 0) { return 2; }
	return 2 - 2 * theYClick / getHeight();
}

GLfloat HarmonicDesigner::GlobalToPhaseY(GLfloat theYClick) {
	if (theYClick > getHeight()) { return -pi+0.001; }
	if (theYClick < 0) { return pi-0.001; }
	return pi - twoPi * theYClick / getHeight();
}

GLfloat HarmonicDesigner::MagnitudeToWorldY(GLfloat mag, int bin) {
	return NormalizeGLY(mag*bin/4, 0, 2);
}

GLfloat HarmonicDesigner::PhaseToWorldY(GLfloat phase) {
	return NormalizeGLY(phase, -pi, pi);
}

GLfloat HarmonicDesigner::NormalizePixelY(GLfloat location, GLfloat min, GLfloat max) {
	GLfloat yHeight = getHeight();
	GLfloat myHeight = max - min;
	return location * yHeight / myHeight + (yMin - min);
}

GLfloat HarmonicDesigner::NormalizeGLY(GLfloat location, GLfloat min, GLfloat max) {
	// scaling
	GLfloat yHeight = yMax - yMin;
	GLfloat myHeight = max - min;
	GLfloat scaling = yHeight / myHeight;

	// translation
	GLfloat centerDistance = (yMax + yMin) / 2 - (max + min)*scaling / 2;

	return location * scaling + centerDistance;
}

//=======================================================================================
// OpenGL render function. Called often, make sure this runs fast.
void HarmonicDesigner::render() {
	if(mySamples == nullptr || !OpenGLHelpers::isContextActive() || calculatingWave) {
		return;
	}

	if (harmonicsChanged) {
		calculateWave();
		harmonicsChanged = false;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(xMin, xMax, yMin, yMax, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor3ub(10, 10, 10);
	glVertex2f(xMin, yMin);
	glVertex2f(xMin, yMax);
	glVertex2f(xMax, yMax);
	glColor3ub(50, 50, 50);
	glVertex2f(xMax, yMin);
	glEnd();

	if (viewingPhases) {
		//------------------------------------
		// Drawing phases:
		//
		// Calculate the phase by taking the
		// arctan of the ratio of the real and
		// imaginary components.
		//
		// Draw the harmonic as a vertical
		// bar wherever the magnitude > 0.
		//
		//------------------------------------
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_QUADS);
		for (int i = 1; i < WAVEFORM_SIZE >> 2; i++) {
			double phase = atan2(myHarmonics[i].imag(), myHarmonics[i].real());
			double im = myHarmonics[i].imag();
			double re = myHarmonics[i].real();
			GLfloat x = (GLfloat)(i << 2);
			GLfloat y = PhaseToWorldY(phase);
			glColor4f(0.5, 0, 10 * x, 0.5);
			glVertex2f(x, -1);
			glVertex2f(x, y);
			glVertex2f(x + 2, y);
			glVertex2f(x + 2, -1);
		}
		glEnd();
	}
	else {
		//------------------------------------
		// Drawing harmonics:
		//
		// Calculate the harmonic amplitudes
		// by taking the absolute value of the
		// complex numbers in each bin.
		//
		// Draw the harmonic as a vertical
		// bar wherever the magnitude > 0.
		//
		//------------------------------------
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_QUADS);
		for (int i = 1; i < WAVEFORM_SIZE >> 2; i++) {
			GLfloat x = (GLfloat)(i << 2);
			double mag = abs(myHarmonics[i]);
			if (mag > 0) {
				GLfloat y = MagnitudeToWorldY(mag, x);
				glColor4f(25 * mag, 0, 10 * x, 0.5);
				glVertex2f(x, -1);
				glVertex2f(x, y);
				glVertex2f(x + 2, y);
				glVertex2f(x + 2, -1);
			}
		}
		glEnd();
	}
	// Render waveform
	glColor3ub(200, 100, 0);
	glLineWidth(1.0f);
	glBegin(GL_LINE_STRIP);
	for (int i = 1; i < WAVEFORM_SIZE; i++) {
		glVertex2f((GLfloat)i, NormalizeGLY(mySamples[i].real(), waveMin, waveMax));
	}
	glEnd();
	glFlush();
}

//=======================================================================================
// Mouse click handlers. Still a lot of work to do on these.
void HarmonicDesigner::mouseUp(const MouseEvent & theEvent) {
	if (theEvent.mods.isRightButtonDown()) {
		viewingPhases = !viewingPhases;
		return;
	}

	modulateHarmonic((GLfloat)theEvent.y, GlobalToHarmonicX((GLfloat)theEvent.x));
}

void HarmonicDesigner::mouseDrag(const MouseEvent & theEvent) {
	// Dragging does nothing for right clicks
	if (theEvent.mods.isRightButtonDown()) {
		return;
	}
	modulateHarmonic((GLfloat)theEvent.y, GlobalToHarmonicX((GLfloat)theEvent.x));

}

void HarmonicDesigner::modulateHarmonic(GLfloat theYClick, int theBin) {
	if (viewingPhases) {
		// modify phase value
		double phase = GlobalToPhaseY(theYClick)-piOverTwo;
		myHarmonics[theBin] = Complex(0, abs(myHarmonics[theBin]));
		myHarmonics[theBin] *= Complex(cos(phase), sin(phase));
	}
	else {
		// modify amplitude value
		double mag = GlobalToMagnitudeY(theYClick) / theBin;
		if (myHarmonics[theBin].real() == 0) {
			myHarmonics[theBin].imag(mag);
		}
		else if (myHarmonics[theBin].imag() == 0) {
			myHarmonics[theBin].real(mag);
		}
		else {
			myHarmonics[theBin] *= (mag / myHarmonics[theBin]);
		}
	}
	sendActionMessage(to_string(theBin) + " " + to_string(myHarmonics[theBin].real()) + " " + to_string(myHarmonics[theBin].imag()));
	harmonicsChanged = true;
}

void HarmonicDesigner::mouseWheelMove(const MouseEvent & theEvent, const MouseWheelDetails & theWheelDetails) {
	ignoreUnused(theEvent);
	//xMax += theWheelDetails.deltaY;
	//xMin -= theWheelDetails.deltaY;
}
//=======================================================================================
// Inherited from OpenGLAppComponent
void HarmonicDesigner::initialise() {}

void HarmonicDesigner::shutdown() {
}
