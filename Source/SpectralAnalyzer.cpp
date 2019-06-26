#include "SpectralAnalyzer.h"

using namespace juce;
using namespace Kyle;

// Constructor
SpectralAnalyzer::SpectralAnalyzer(Queue<double>::Ptr theBuffer) : OpenGLAppComponent(),
		myBufferPointer(theBuffer) {
	readyToRender = false;
	mySamples.allocate(fftSize, true);
	fftData.allocate(fftSize, true);
	mySampleRate = 0;
	bufferReadPos = 0;
	sampleIndex = 0;
	startTimerHz(60);
}

SpectralAnalyzer::~SpectralAnalyzer() {
	openGLContext.detach();
	shutdownOpenGL();
}

void SpectralAnalyzer::setSampleRate(int theSampleRate) {
	mySampleRate = theSampleRate;
	binWidth = (double)mySampleRate / (double)fftSize;
}

void SpectralAnalyzer::initialise() {}

void SpectralAnalyzer::shutdown() {
}

void SpectralAnalyzer::clear() {}

void SpectralAnalyzer::resized() {
	//repaint();
}

void SpectralAnalyzer::paint(Graphics & g) {
	/*g.setColour(Colours::white);
	g.setFont(15);
	g.drawText("Spectral Analyzer", 25, 20, 320, 30, Justification::left);
	g.drawLine(20, 20, 220, 20);
	g.drawLine(20, 50, 220, 50);*/
}

// OpenGL render function. Keep this one efficient!
void SpectralAnalyzer::render() {
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(xMin, xMax, yMin, yMax, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw the background
	glBegin(GL_QUADS);
	glColor3ub(10, 10, 10);
	glVertex2f(xMin, yMin);
	glVertex2f(xMin, yMax);
	glVertex2f(xMax, yMax);
	glColor3ub(50, 50, 50);
	glVertex2f(xMax, yMin);
	glEnd();

	// If this is true, our sound is not initialized yet, so bail
	if(mySampleRate == 0) {
		return;
	}

	// FFT the thing if we have enough samples
	if (readyToRender) {
		runFFT();
		readyToRender = false;
	}

	// Draw horizontal line
	/*glBegin(GL_LINE_STRIP);
	glColor3f(1.f, 1.f, 1.f);
	for (int i = 0; i < (scopeSize); i++) {
		double r = fftData[i].real();
		double im = fftData[i].imag();
		GLfloat x = log2(i*binWidth);
		GLfloat level = 10*log10(r*r + im * im);

		glVertex2f(x, max(level,0));

		//for (int j = 0; j < fftSize / 4; j++)
		//	DBG(fftData[j].real()*fftData[j].real() + fftData[j].imag()*fftData[j].imag());
	}
	glEnd();*/
	
	// Draw vertical lines
	for (int i = 1; i < (scopeSize); i++) {
		double r = fftData[i].real();
		double im = fftData[i].imag();
		GLfloat x = log2(i*binWidth);
		GLfloat level = pow(max(0,log10(r*r + im*im))/4.f,4);
		if (level > 0.01) {
			//DBG(level);
			glBegin(GL_LINES);
			glColor3f(level, 0.7*level, 0.7*(1-level));
			glVertex2f(x, yMin);
			glVertex2f(x, yMax);
			glEnd();
		}
	}
	glFlush();
}

void SpectralAnalyzer::timerCallback() {
	if (myBufferPointer == nullptr) { return; }
	while (myBufferPointer->NumToRead(bufferReadPos) > 0) {
		if (sampleIndex == fftSize) {
			if (!readyToRender) {
				fftData.clear(fftSize);
				for (int i = 0; i < fftSize; i++) {
					fftData[i] = mySamples[i];
				}
				readyToRender = true;
			}
			sampleIndex = 0;
		}
		mySamples[sampleIndex++] = myBufferPointer->Read(bufferReadPos);
	}
}

// Run Fast Fourier Transform - Convert from time domain to frequency domain
void SpectralAnalyzer::runFFT() {
	for (int i = 0; i < fftSize; i++) {
		double multiplier = 0.5 * (1 - cos(2 * pi*i / fftSize));		//hann window 
		//double multiplier = 0.42 - 0.5*cos((2 * pi*i) / (fftSize))	+ .076*cos((4 * pi*i) / (fftSize)); //blackman window
		fftData[i] = multiplier * fftData[i];
	}
	Kyle::FFT::DoItForward(fftData.getData(), fftSize);
}
