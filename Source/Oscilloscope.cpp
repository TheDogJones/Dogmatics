#include "Types.h"
#include "Oscilloscope.h"

using namespace juce;
using namespace Kyle;

// Constructor
Oscilloscope::Oscilloscope(DogmaticsAudioProcessorEditor &p) : parent(p) {
	myBuffer = parent.getBuffer();
	bufferReadPos = 0;
}

Oscilloscope::~Oscilloscope() {
	openGLContext.detach();
	shutdownOpenGL();
}

void Oscilloscope::initialise() {
}

void Oscilloscope::shutdown() {
}

void Oscilloscope::clear() {}

void Oscilloscope::resized() {
}

// Draw text in top left
void Oscilloscope::paint(Graphics & g) {
	g.setColour(Colours::white);
	g.setFont(20);
	g.drawText("Oscilloscope", g.getClipBounds(), Justification::topRight);
}

// OpenGL render function. Keep this one super efficient!
void Oscilloscope::render() {
	const GLfloat width = 256;
	const GLfloat halfWidth = 128;
	
	// Mark our location in read buffer
	// myReadPos = (myReadPos + mySamplesReceived)&BUFFERMASK;
	// mySamplesReceived = 0;

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-halfWidth, halfWidth, -yMax, yMax, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor3ub(10, 10, 10);
	glVertex2f(-halfWidth, -yMax);
	glVertex2f(-halfWidth, yMax);
	glVertex2f(halfWidth, yMax);
	glColor3ub(50, 50, 50);
	glVertex2f(halfWidth, -yMax);
	glEnd();

	// Render lines
	if (myBuffer->NumToRead(bufferReadPos) > width) {
		glDeleteLists(list, 1);
		list = glGenLists(1);
		glNewList(list, GL_COMPILE);
		glColor3ub(200, 100, 0);
		glBegin(GL_LINE_STRIP);
		int i = 0;
		while (myBuffer->NumToRead(bufferReadPos) > 0) {
			glVertex2f((GLfloat)i - halfWidth, 4.0f*(GLfloat)myBuffer->Read(bufferReadPos));
			i++;
		}
		glEnd();
		glEndList();
	}

	glCallList(list);

	glFlush();
}
