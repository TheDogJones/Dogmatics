#include "Types.h"
#include "HarmonicDesigner.h"

using namespace juce;
using namespace Kyle;

//=======================================================================================
// Constructor
HarmonicDesigner::HarmonicDesigner(DogmaticsAudioProcessorEditor &p) : parent(p) {
	
	myHarmonics.allocate(WAVEFORM_SIZE, true);
	Sine(myHarmonics, WAVEFORM_SIZE);
	
	calculateWave();
}

void HarmonicDesigner::mouseMove(const MouseEvent & theEvent) {
	//Component::repaint();
}

HarmonicDesigner::~HarmonicDesigner() {
	myHarmonics.free();
	mySamples.free();
	openGLContext.detach();
	shutdownOpenGL();
}
//=======================================================================================

void HarmonicDesigner::resized() {
	//Component::setBoundsInset(BorderSize(0));
}

// repaint. write the text in the top left
void HarmonicDesigner::paint(Graphics & g) {
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

	//glDeleteLists(list, 1);
	//glNewList(list, GL_COMPILE);

	//glEndList();
	calculatingWave = false;
}

//void HarmonicDesigner::mouseMove(const MouseEvent & theEvent) {
	//OpenGLAppComponent::repaint();
//}

//=======================================================================================
// Conversion functions for converting x and y positions relative to screen to
// OpenGL coordinates
GLfloat HarmonicDesigner::GlobalToWorldX(GLfloat theXClick) {
	return theXClick * (xMax / getWidth());
}

int HarmonicDesigner::GlobalToHarmonicX(GLfloat theXClick) {
	return (int)floorl(GlobalToWorldX(theXClick) / 4);
}

GLfloat HarmonicDesigner::GlobalToMagnitudeY(GLfloat theYClick) {
	if (theYClick > getHeight()) { return 0; }
	if (theYClick < 0) { return 2; }
	return 2-2*(theYClick / getHeight());
}

GLfloat HarmonicDesigner::MagnitudeToWorldY(GLfloat mag, int bin) {
	return NormalizeGLY(mag*bin/4, 0, 2);
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
	GLfloat centerDistance = (yMax + yMin) / 2 - (max + min) / 2;

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

	//------------------------------------
	// Drawing harmonics:
	//
	// Each frequency bin x contains:
	//
	//  re{x} = double*cos*(phase)
	//  im{x} = double*sin*(phase)
	//
	// Draw the double as a vertical
	// bar wherever the magnitude > 0
	//
	// Phase will require some arctan shit
	//------------------------------------
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	for (int i = 1; i < WAVEFORM_SIZE >> 2; i++) {
		double mag = std::sqrt(myHarmonics[i].real()*myHarmonics[i].real() + myHarmonics[i].imag()*myHarmonics[i].imag());
		GLfloat x = (GLfloat)(i << 2);
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
	if (theEvent.x > xMax || theEvent.x < xMin) { return; }
	int bin = GlobalToHarmonicX((GLfloat)theEvent.x);
	if (bin == 0) { return; }
	double mag = GlobalToMagnitudeY((GLfloat)theEvent.y)/bin;
	myHarmonics[bin].imag(mag);
	sendActionMessage(to_string(bin) + " " + to_string((mag)));
	harmonicsChanged = true;
}

void HarmonicDesigner::mouseDrag(const MouseEvent & theEvent) {
	int bin = GlobalToHarmonicX((GLfloat)theEvent.x);
	if (bin == 0) { return; }
	double mag = GlobalToMagnitudeY((GLfloat)theEvent.y) / bin;
	myHarmonics[bin].imag(mag);
	sendActionMessage(to_string(bin) + " " + to_string((mag)));
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
