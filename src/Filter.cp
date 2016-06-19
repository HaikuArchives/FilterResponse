// Filter.cp

#include "Filter.h"
#include "FRApp.h"
#include "FRNumbersWindow.h"
#include "FRPrefs.h"
#include <math.h>

char *gCoefficientLabels[] = {
	"a[0]:",
	"a[1]:",
	"a[2]:",
	"b[1]:",
	"b[2]:"
};
double FilterCoefficients::a0 = 1.0;
double FilterCoefficients::a1 = 0.0;
double FilterCoefficients::a2 = 0.0;
double FilterCoefficients::b1 = 0.0;
double FilterCoefficients::b2 = 0.0;
double FilterButterBand2a::cLowFreq = 500.0;
double FilterButterBand2a::cHighFreq = 2000.0;
double FilterButterBand2b::cCentreFreq = 1000.0;
double FilterButterBand2b::cQ = 1.0;
float Filter::x[2][MAX_FILTER_SIZE];
float Filter::y[2][MAX_FILTER_SIZE];
int Filter::index = MAX_FILTER_SIZE;
float *Filter::xp[2] = { &Filter::x[0][0] - 1, &Filter::x[1][0] - 1 };
float *Filter::yp[2] = { &Filter::y[0][0] - 1, &Filter::y[1][0] - 1 };

Filter::Filter(int32 kind, double sample_rate, int sizeA, int sizeB)
{
	fFilterKind = kind;
	fSampleRate = sample_rate;
	fFilterSizeA = sizeA;
	fFilterSizeB = sizeB;
	fSamplePeriod = 1.0 / sample_rate;
	a[0] = 1.0;
	a[1] = 0.0;
	a[2] = 0.0;
	b[0] = 1.0;
	b[1] = 0.0;
	b[2] = 0.0;
	pi = atan(1.0) * 4.0;
	twopi = pi * 2.0;
}

void Filter::Response(double frequency, double *amplitude, double *phase)
{
	int i;
	double j,q;
	double sum1_re, sum1_im, sum2_re, sum2_im, temp_re, temp_im;
	
	sum1_re = sum1_im = 0.0;
	sum2_re = sum2_im = 0.0;
	temp_re = temp_im = 0.0;
	
	for(i = 0; i < fFilterSizeA; i++)
	{
		j = i;
		q = -twopi * fSamplePeriod * j * frequency;
		temp_re = a[i] * cos(q);
		temp_im = a[i] * sin(q);
		sum1_re += temp_re;
		sum1_im += temp_im;
	}
	for(i = 1; i < fFilterSizeB; i++)		// don't need b[0], since it's always 1.0
	{
		j = i;
		q = -twopi * fSamplePeriod * j * frequency;
		temp_re = b[i] * cos(q);
		temp_im = b[i] * sin(q);
		sum2_re += temp_re;
		sum2_im += temp_im;
	}
	double a,b,c,d,z;
	a = sum1_re;
	b = sum1_im;
	c = 1.0 - sum2_re;
	d = 0.0 - sum2_im;
	z = c*c + d*d;
	temp_re = (a*c + b*d) / z;
	temp_im = (b*c - a*d) / z;
	*amplitude = sqrt(temp_re*temp_re + temp_im*temp_im);
	*phase = atan2(temp_im, temp_re);
}

void Filter::AdjustNumbersWindow(FRNumbersWindow *window, bool showHide)
{
	// do nothing
}

void Filter::NumberChanged(int index, double value)
{
	// do nothing
}

void Filter::CalculateCoefficients()
{
	// do nothing
}

void Filter::Process(float **inputs, float **outputs, int count)
{
	if (fFilterSizeA == 3 && fFilterSizeB == 3)
	{
		// what do you mean you don't have this many registers??
		register float x0L,x1L,x2L,y0L,y1L,y2L;
		register float x0R,x1R,x2R,y0R,y1R,y2R;
		register double a0,a1,a2,b1,b2;
		register float *inL = inputs[0] - 1;
		register float *inR = inputs[1] - 1;
		register float *outL = outputs[0] - 1;
		register float *outR = outputs[1] - 1;
		register int i = index, c;
		register float *xp0 = xp[0];
		register float *xp1 = xp[1];
		register float *yp0 = yp[0];
		register float *yp1 = yp[1];
		// localise the coefficients & previous samples
		a0 = a[0];
		a1 = a[1];
		a2 = a[2];
		b1 = b[1];
		b2 = b[2];
		if (i >= MAX_FILTER_SIZE) c = MAX_FILTER_SIZE;
		else c = 0;
		x1L = xp0[c];
		x1R = xp1[c];
		y1L = yp0[c];
		y1R = yp1[c];
		if (i >= MAX_FILTER_SIZE - 1) c = MAX_FILTER_SIZE - 1;
		else c = -1;
		x2L = xp0[c];
		x2R = xp1[c];
		y2L = yp0[c];
		y2R = yp1[c];
		
		while (count > 0)
		{
			if (i < count) c = index;
			else c = count;
			count -= c;
			i -= c;
			for(;--c>=0;)
			{
				// read a sample
				x0L = *++inL;
				x0R = *++inR;
				// calculate the output sample
				y0L = a0*x0L + a1*x1L + a2*x2L + b1*y1L + b2*y2L;
				y0R = a0*x0R + a1*x1R + a2*x2R + b1*y1R + b2*y2R;
				// write a sample
				*++outL = y0L;
				*++outR = y0R;
				// keep a record of our current samples
				*++xp0 = x0L;
				*++xp1 = x0R;
				*++yp0 = y0L;
				*++yp1 = y0R;
				// shift the data in our registers
				y2L = y1L;
				y2R = y1R;
				x2L = x1L;
				x2R = x1R;
				y1L = y0L;
				y1R = y0R;
				x1L = x0L;
				x1R = x0R;
			}
			if (i <= 0)
			{
				i += MAX_FILTER_SIZE;
				xp0 -= MAX_FILTER_SIZE;
				xp1 -= MAX_FILTER_SIZE;
				yp0 -= MAX_FILTER_SIZE;
				yp1 -= MAX_FILTER_SIZE;
			}
		}
		xp[0] = xp0;
		xp[1] = xp1;
		yp[0] = yp0;
		yp[1] = yp1;
		index = i;
	}
	else
	{
		int i = 0;
		i = i + 1;
	}
}

FilterCoefficients::FilterCoefficients(double sample_rate)
	: Filter(filterCoefficients, sample_rate)
{
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
	b[0] = 1.0;
	b[1] = b1;
	b[2] = b2;
}

FilterCoefficients::~FilterCoefficients()
{
	a0 = a[0];
	a1 = a[1];
	a2 = a[2];
	b1 = b[1];
	b2 = b[2];
}

void FilterCoefficients::AdjustNumbersWindow(FRNumbersWindow *window, bool showHide)
{
	int i;
	if (showHide)
	{
		char str[256];
		float f = 0.0;
		double *array = a;
		for(i = 0; i < 5; i++)
		{
			if (i == 3) array = &b[-2];
			window->fText[i]->SetLabel(gCoefficientLabels[i]);
			sprintf(str,"%1.5f",array[i]);
			window->fText[i]->SetText(str);
			window->fText[i]->Show();
		}
	}
	else
	{
		for(i = 0; i < 5; i++)
			window->fText[i]->Hide();
	}
}

void FilterCoefficients::NumberChanged(int index, double value)
{
	if (index < 3)
		a[index] = value;
	else if ((index -= 3) < 2) {
		b[index + 1] = value;
	}
}


FilterLowPass::FilterLowPass(double sample_rate)
	: Filter(filterLowPass, sample_rate)
{
	double freq;
	gPreferences->FindDouble("Last Frequency",&freq);
	fFrequency = freq;
}

FilterLowPass::~FilterLowPass()
{
	uint32 m = modifiers();
	if (m & B_COMMAND_KEY || m & B_SHIFT_KEY)
	{
		FilterCoefficients::a0 = a[0];
		FilterCoefficients::a1 = a[1];
		FilterCoefficients::a2 = a[2];
		FilterCoefficients::b1 = b[1];
		FilterCoefficients::b2 = b[2];
	}
	gPreferences->ReplaceDouble("Last Frequency",fFrequency);
}

void FilterLowPass::AdjustNumbersWindow(FRNumbersWindow *window, bool showHide)
{
	if (showHide)
	{
		window->fText[0]->SetLabel("Frequency:");
		char str[256];
		sprintf(str,"%1.3f",(float)fFrequency);
		window->fText[0]->SetText(str);
		window->fText[0]->Show();
	} else {
		window->fText[0]->Hide();
	}
}

void FilterLowPass::NumberChanged(int index, double d)
{
	if (index != 0) return;
	fFrequency = d;
}

void FilterLowPass::CalculateCoefficients()
{
	a[1] = 0.0;
	a[2] = 0.0;
	b[0] = 1.0;
	b[2] = 0.0;
	double x = fFrequency * twopi / fSampleRate;
	a[0] = x;
	b[1] = 1 - x;
}


FilterButterBand2a::FilterButterBand2a(double sample_rate)
	: Filter(filterButterBand2a, sample_rate)
{
	fLowFreq = cLowFreq;
	fHighFreq = cHighFreq;
}

FilterButterBand2a::~FilterButterBand2a()
{
	cLowFreq = fLowFreq;
	cHighFreq = fHighFreq;
	if (modifiers() & B_COMMAND_KEY || modifiers() & B_SHIFT_KEY)
	{
		FilterCoefficients::a0 = a[0];
		FilterCoefficients::a1 = a[1];
		FilterCoefficients::a2 = a[2];
		FilterCoefficients::b1 = b[1];
		FilterCoefficients::b2 = b[2];
		double c = sqrt(fLowFreq * fHighFreq);
		FilterButterBand2b::cCentreFreq = c;
		FilterButterBand2b::cQ = c / (fHighFreq - fLowFreq);
	}
}

void FilterButterBand2a::AdjustNumbersWindow(FRNumbersWindow *window, bool showHide)
{
	if (showHide)
	{
		window->fText[0]->SetLabel("Low Frequency:");
		window->fText[1]->SetLabel("High Frequency:");
		char str[256];
		sprintf(str,"%1.3f",fLowFreq);
		window->fText[0]->SetText(str);
		sprintf(str,"%1.3f",fHighFreq);
		window->fText[1]->SetText(str);
		window->fText[0]->Show();
		window->fText[1]->Show();
	} else {
		window->fText[0]->Hide();
		window->fText[1]->Hide();
	}
}

void FilterButterBand2a::NumberChanged(int index, double value)
{
	switch (index)
	{
	case 0:
		fLowFreq = value;
		break;
	case 1:
		fHighFreq = value;
		break;
	}
}

void FilterButterBand2a::CalculateCoefficients()
{
	double v1 = fLowFreq;
	double v3 = fHighFreq;
	v1 /= fSampleRate/2;
	v3 /= fSampleRate/2;
	double D = 1 / tan(pi*(v3-v1)/2);
	double E = 2 * cos(pi*(v3+v1)/2) / cos(pi*(v3-v1)/2);
	double A = 1 + D;
	a[0] = 1/A;
	a[1] = 0;
	a[2] = -1/A;
	b[1] = D * E / A;
	b[2] = (1 - D) / A;
}


FilterButterBand2b::FilterButterBand2b(double sample_rate)
	: Filter(filterButterBand2b, sample_rate)
{
	fCentreFreq = cCentreFreq;
	fQ = cQ;
}

FilterButterBand2b::~FilterButterBand2b()
{
	cCentreFreq = fCentreFreq;
	cQ = fQ;
	if (modifiers() & B_COMMAND_KEY || modifiers() & B_SHIFT_KEY)
	{
		FilterCoefficients::a0 = a[0];
		FilterCoefficients::a1 = a[1];
		FilterCoefficients::a2 = a[2];
		FilterCoefficients::b1 = b[1];
		FilterCoefficients::b2 = b[2];
		double c = 1 / fQ;
		double d = c + sqrt(c*c + 4);
		d /= 2.0;
		FilterButterBand2a::cLowFreq = fCentreFreq / d;
		FilterButterBand2a::cHighFreq = fCentreFreq * d;
	}
}

void FilterButterBand2b::AdjustNumbersWindow(FRNumbersWindow *window, bool showHide)
{
	if (showHide)
	{
		window->fText[0]->SetLabel("Frequency:");
		window->fText[1]->SetLabel("Q Factor:");
		char str[256];
		sprintf(str,"%1.3f",fCentreFreq);
		window->fText[0]->SetText(str);
		sprintf(str,"%1.3f",fQ);
		window->fText[1]->SetText(str);
		window->fText[0]->Show();
		window->fText[1]->Show();
	} else {
		window->fText[0]->Hide();
		window->fText[1]->Hide();
	}
}

void FilterButterBand2b::NumberChanged(int index, double value)
{
	switch (index)
	{
	case 0:
		fCentreFreq = value;
		break;
	case 1:
		fQ = value;
		break;
	}
}

void FilterButterBand2b::CalculateCoefficients()
{
	double c = 1 / fQ;
	double d = sqrt(c*c + 4.0) + c;
	d = d / 2.0;
	double v1 = fCentreFreq / d;
	double v3 = fCentreFreq * d;
	v1 /= fSampleRate/2;
	v3 /= fSampleRate/2;
	double D = 1 / tan(pi*(v3-v1)/2);
	double E = 2 * cos(pi*(v3+v1)/2) / cos(pi*(v3-v1)/2);
	double A = 1 + D;
	a[0] = 1/A;
	a[1] = 0;
	a[2] = -1/A;
	b[1] = D * E / A;
	b[2] = (1 - D) / A;
}
