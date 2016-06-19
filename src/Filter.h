// Filter.h

#pragma once

#include "Benaphore.h"

#define MAX_FILTER_SIZE 512

class FRNumbersWindow;

class Filter
{
public:
	Filter(int32 kind, double sample_rate, int sizeA = 3, int sizeB = 3);
	virtual ~Filter() {}
	
	void Response(double frequency, double *amplitude, double *phase);
	virtual void AdjustNumbersWindow(FRNumbersWindow *window, bool showHide);
	virtual void NumberChanged(int index, double value);
	virtual void CalculateCoefficients();
	
	void Process(float **inputs, float **outputs, int count);
	
	double fSampleRate;
	double fSamplePeriod;
	int fFilterSizeA, fFilterSizeB;
	double a[MAX_FILTER_SIZE], b[MAX_FILTER_SIZE];
static float x[2][MAX_FILTER_SIZE], y[2][MAX_FILTER_SIZE];
static int index;
static float *xp[2], *yp[2];
	double pi, twopi;
	int32 fFilterKind;
};

class FilterCoefficients : public Filter
{
	
public:
	FilterCoefficients(double sample_rate);
	virtual ~FilterCoefficients();
	
	virtual void AdjustNumbersWindow(FRNumbersWindow *window, bool showHide);
	virtual void NumberChanged(int index, double value);
	
	static double a0,a1,a2,b1,b2;
};

class FilterLowPass : public Filter
{
	double fFrequency;
	
public:
	FilterLowPass(double sample_rate);
	virtual ~FilterLowPass();
	
	virtual void AdjustNumbersWindow(FRNumbersWindow *window, bool showHide);
	virtual void NumberChanged(int index, double value);
	virtual void CalculateCoefficients();
};

class FilterButterBand2a : public Filter
{
public:
	FilterButterBand2a(double sample_rate);
	virtual ~FilterButterBand2a();
	
	virtual void AdjustNumbersWindow(FRNumbersWindow *window, bool showHide);
	virtual void NumberChanged(int index, double value);
	virtual void CalculateCoefficients();
	
	double fLowFreq, fHighFreq;
	static double cLowFreq, cHighFreq;
};

class FilterButterBand2b : public Filter
{
public:
	FilterButterBand2b(double sample_rate);
	virtual ~FilterButterBand2b();
	
	virtual void AdjustNumbersWindow(FRNumbersWindow *window, bool showHide);
	virtual void NumberChanged(int index, double value);
	virtual void CalculateCoefficients();
	
	double fCentreFreq, fQ;
	static double cCentreFreq, cQ;
};


