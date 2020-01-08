#pragma once

#ifndef KYLE
#define KYLE

#include "../JuceLibraryCode/JuceHeader.h"
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <math.h>
#include <complex>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <limits>
#include <chrono> 

namespace Kyle
{
	#define pi 3.14159265358
	#define piOverTwo 1.57079632679
	#define twoPi 6.28318530717
	#define twelfthRootTwo 1.05946309435

	#define WAVEFORM_SIZE 512
	#define OVERSAMPLE_FACTOR 1
	#define FLOATTYPE double

	const int BUFFER = 16 * WAVEFORM_SIZE;
	const int BUFFERMASK = BUFFER - 1;
	const int WAVEFORM_LOG = (int)log2(WAVEFORM_SIZE);

	using namespace std;

	// Custom data types
	//==============================================
	typedef complex<FLOATTYPE> Complex;

	// Basic wave functions
	//==============================================
	typedef function<void(Complex*, int)> Wave;
	
	static Wave Sine = [](auto *samples, auto size) -> auto {
		samples[1] = Complex(0, 1);
		for (int i = 2; i < size / 2; i++) {
			samples[i] = Complex(0, 0);
		}
	};

	static Wave Square = [](auto *samples, auto size) -> auto {
		for (int i = 1; i < size / 2; i++) {
			samples[i] = Complex(0, i % 2 == 0 ? 0 : 1 / (double)i);
		}
	};

	static Wave Triangle = [](auto *samples, auto size) -> auto {
		for (int i = 1; i < size / 2; i++) {
			samples[i] = i % 2 == 0 ? Complex(0)
									: i % 4 == 3 ? Complex(0, -1 / (double)(i*i))
										         : Complex(0,  1 / (double)(i*i));
				
		}
	};

	static Wave Saw = [](auto *samples, auto size) -> auto {
		samples[0] = Complex(0, 0);
		Complex current;
		for (int i = 1; i < size / 2; i++) {
			current = Complex(0, 1 / (double)i);
			samples[i] = current;
		}
		samples[size - 1] = Complex(0, 0);
	};

	// Blackman window for filter design
	static Wave Blackman = [](auto *samples, auto size) -> auto {
		for (int i = 0; i < size; i++) {
			samples[i] = 0.42 - 0.5*cos((2 * pi*i) / (size))
				+ .076*cos((4 * pi*i) / (size));
		}
	};
	
	// Utility functions
	//=================================================

	static inline double NoteToFrequency(int theOctave, int theNote) {
		return double((440 * pow(2.0, (((theOctave - 4) * 12 + theNote)) / 12.0)));
	}

	static inline void NormalizeSamples(double *theSamples, int theSize) {
		double max = 0.0;
		for (int i = 0; i < theSize; i += 1) {
			if (abs(theSamples[i]) > max) {
				max = abs(theSamples[i]);
			}
		}
		for (int i = 0; i < theSize; i++) {
			theSamples[i] = theSamples[i] / max;
		}
	}

	// t is a value that goes from 0 to 1 to interpolate in a C1 continuous way across uniformly sampled data points.
	// when t is 0, this will return B.  When t is 1, this will return C.
	static double CubicHermite(double A, double B, double C, double D, double t) {
		double a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
		double b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
		double c = -A / 2.0f + C / 2.0f;
		double d = B;

		return a * t*t*t + b * t*t + c * t + d;
	}

	// Return int between zero and max size
	static int GetIndexClamped(int theInput, int theSize) {
		if (theInput < 0)
			return theInput + theSize;
		else if (theInput >= theSize)
			return theInput % theSize;
		else
			return theInput;
	}

	namespace FFT
	{
		// doin it forwards
		static void DoItForward(Complex *theSamples, unsigned int theSize) {
			// DFT
			unsigned int N = theSize, k = N, n;
			double thetaT = pi / N;
			Complex phiT = Complex(cos(thetaT), sin(thetaT)), T;
			while (k > 1)
			{
				n = k;
				k >>= 1;
				phiT = phiT * phiT;
				T = 1.0L;
				for (unsigned int l = 0; l < k; l++)
				{
					for (unsigned int a = l; a < N; a += n)
					{
						unsigned int b = a + k;
						Complex t = theSamples[a] - theSamples[b];
						theSamples[a] = theSamples[a] + theSamples[b];
						theSamples[b] = t * T;
					}
					T *= phiT;
				}
			}
			// Decimate
			unsigned int m = (unsigned int)log2(N);
			for (unsigned int a = 0; a < N; a++)
			{
				unsigned int b = a;
				// Reverse bits
				b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
				b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
				b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
				b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
				b = ((b >> 16) | (b << 16)) >> (32 - m);
				if (b > a)
				{
					Complex t = theSamples[a];
					theSamples[a] = theSamples[b];
					theSamples[b] = t;
				}
			}
		}

		// doin it backwards
		static void DoItBackwards(Complex *theSamples, unsigned int theSize) {
			// conjugate the complex numbers
			for (unsigned int i = 0; i < theSize; i++) {
				theSamples[i].imag(-theSamples[i].imag());
			}

			// forward fft
			DoItForward(theSamples, theSize);

			// conjugate the complex numbers again
			for (unsigned int i = 0; i < theSize; i++) {
				theSamples[i].imag(-theSamples[i].imag());
			}
		}
	};

	// Interpolate
	static double CubicInterpolate(Complex *theSamples, int theSize, double thePos, bool theUseImag = false) {
		int pos = (int)thePos;
		double t = thePos - pos;
		double a = theUseImag ? theSamples[GetIndexClamped(pos - 1, theSize)].imag() : theSamples[GetIndexClamped(pos - 1, theSize)].real();
		double b = theUseImag ? theSamples[GetIndexClamped(pos, theSize)].imag() : theSamples[GetIndexClamped(pos, theSize)].real();
		double c = theUseImag ? theSamples[GetIndexClamped(pos + 1, theSize)].imag() : theSamples[GetIndexClamped(pos + 1, theSize)].real();
		double d = theUseImag ? theSamples[GetIndexClamped(pos + 2, theSize)].imag() : theSamples[GetIndexClamped(pos + 2, theSize)].real();
		return CubicHermite(a, b, c, d, t);
	}


	// Fast FIFO queue class, designed for oscillators and other things that move data in and out quickly.
	template<typename T>
	class Queue : public ReferenceCountedObject {
	public:
		using Ptr = ReferenceCountedObjectPtr<Queue<T>>;
		Queue<T>() {
			myArray.allocate(BUFFER, false);
			myWritePointer = 0;
			myReadPointer = 0;
			overflows = 0;
		}
		~Queue<T>() {
			//myArray.free();
		}

		class Index {
		public:
			Index() {
				readIndex = 0;
				overflows = 0;
			}
			int readIndex;
			int overflows;
		};

		HeapBlock<T> myArray;
		int myWritePointer, myReadPointer;
		int overflows;
		bool isFull;

		T& operator[] (const int theIndex) {
			return myArray[theIndex];
		}
		void Push(const T theValue) {
			myArray[myWritePointer] = theValue;
			myWritePointer++;
			if (myWritePointer == BUFFER) {
				myWritePointer = 0;
				overflows++;
			}
		}
		T Read() {
			T tmp = myArray[myReadPointer];
			myReadPointer = (myReadPointer + 1) & BUFFERMASK;
			return tmp;
		}
		T Read(int &theIndex) {
			T tmp = myArray[theIndex];
			theIndex = (theIndex + 1) & BUFFERMASK;
			return tmp;
		}
		T Read(Index &thePtr) {
			T tmp = myArray[thePtr.readIndex];
			thePtr.readIndex++;
			if (thePtr.readIndex == BUFFER) {
				thePtr.readIndex = 0;
				thePtr.overflows++;
			}
			return tmp;
		}
		int NumToRead() const {
			return (myWritePointer - myReadPointer + BUFFER) & BUFFERMASK;
		}
		int NumToRead(int theIndex) const {
			return (myWritePointer - theIndex + BUFFER) & BUFFERMASK;
		}
		int NumToRead(Index thePtr) const {
			return (myWritePointer - thePtr.readIndex) + BUFFER*(overflows-thePtr.overflows);
		}
	};

	template<typename T>
	T* HeapBlockToArray(HeapBlock<T> original) {
		return (T*)(original.getData());
	}

	template<typename T>
	T* HeapBlockToArray(HeapBlock<T> *original) {
		return (T*)(original->getData());
	}

	static void timeFunction(function<void()> func, string name) {
		auto start = chrono::high_resolution_clock::now();
		func();
		auto stop = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
		DBG(name << " took " << duration.count() << " microseconds.");
	}

	static void p(HeapBlock<Complex> h, int i) { DBG(h[i].real() << " " << h[i].imag()); }

	static void testFFT() {
		HeapBlock<Complex> h;
		h.allocate(WAVEFORM_SIZE, true);
		for (int i = 0; i < WAVEFORM_SIZE; i++) {
			h[i].real(sin((double)i / (double)WAVEFORM_SIZE * 4.0 * pi));
			DBG(i << ": " << h[i].real() << " " << h[i].imag());
		}
		FFT::DoItForward(h.getData(), WAVEFORM_SIZE);
		for (int i = 0; i < WAVEFORM_SIZE; i++) {
			DBG(i << ": " << h[i].real() << " " << h[i].imag());
		}
	}
}

// Prototypes
class DogmaticsAudioProcessor;
class DogmaticsAudioProcessorEditor;
class HarmonicDesigner;
class Oscilloscope;
class SpectralAnalyzer;
class EffectsPanel;

#ifdef JUCE_DEBUG
class AdvancedLeakDetector {
public:
	AdvancedLeakDetector() {
		getBackTraceHash().set((void *)this, SystemStats::getStackBacktrace());
	}
	~AdvancedLeakDetector() {
		getBackTraceHash().remove((void *)this);
	}
private:
	typedef HashMap<void*, String> BackTraceHash;
	struct HashHolder {
		~HashHolder() {
			if (traces.size() > 0)
			{
				/* Memory leak info. */
				DBG("Found " + String(traces.size()) + " possible leaks");
				for (BackTraceHash::Iterator i(traces); i.next();)
				{
					DBG("-----");
					DBG(i.getValue());
				}
				jassertfalse;
			}
		}
		BackTraceHash traces;
	};
	BackTraceHash & getBackTraceHash() {
		static HashHolder holder;
		return holder.traces;
	}
};
#endif

#endif //KYLE