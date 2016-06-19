// FRDisplayWindow.cp

#include "FRDisplayWindow.h"
#include "FRPrefs.h"
#include "FRApp.h"
#include "Filter.h"

FRDisplayWindow::FRDisplayWindow(BRect rect)
	: BWindow(rect, "Filter Response",
		B_DOCUMENT_WINDOW, 0)
{
	Lock();
	
	SetSizeLimits(320, 1024, 240, 768);
	SetZoomLimits(1024, 768);
	
	BRect r = Bounds();
	
	fGraph = new ResponseGraph(r);
	AddChild(fGraph);
	
	Unlock();
}

void FRDisplayWindow::Quit()
{
	BRect r = Frame();
	gPreferences->ReplaceRect("Display Window", r);
	
	BWindow::Quit();
}

void FRDisplayWindow::MessageReceived(BMessage *message)
{
	BWindow::MessageReceived(message);
}

bool FRDisplayWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(false);		// let the application thread kill us
}



ResponseGraph::ResponseGraph(BRect r)
	: BView(r, "Response",
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
{
	minFrequency = 20.0;
	maxFrequency = 20000.0;
	minAmplitude = 0.5;
	maxAmplitude = 2.0;
	left = 33;
	top = 17;
	r = Bounds();
	FrameResized(r.right, r.bottom);
	
	amplitudes = (double*) malloc(1024 * sizeof(double));
	if (!amplitudes) debugger("Couldn't malloc for amplitudes\n");
	phases = (double*) malloc(1024 * sizeof(double));
	if (!phases) debugger("Couldn't malloc for phases\n");
	
	double *a = amplitudes - 1, *p = phases - 1;
	for(int i = 0; i < 1024; i++)
	{
		*++a = 1.0;
		*++p = 0.0;
	}
	pi = atan(1.0) * 4.0;
}

ResponseGraph::~ResponseGraph()
{
	free(amplitudes);
	amplitudes = NULL;
	free(phases);
	phases = NULL;
}


void ResponseGraph::Draw(BRect area)
{
	DrawGrid();
	PlotGraph();
}

void ResponseGraph::FrameResized(float x, float y)
{
	width = x - 66;
	height = y - 34;
}

void ResponseGraph::DrawGrid()
{
	SetHighColor(0,0,0);
	SetLowColor(255,255,255);
	BRect r = Bounds();
	r.InsetBy(32,16);
	StrokeRect(r);
	r.InsetBy(1,1);
	FillRect(r,B_SOLID_LOW);
	SetHighColor(128,128,128);
	BPoint a(left,amp2y(1.0));
	BPoint b(left+width,a.y);
	StrokeLine(a,b);
	SetHighColor(230,230,230);
	double amp = 1.0;
	for(amp = 2.0; amp < maxAmplitude; amp *= 2)
	{
		b.y = a.y = amp2y(amp);
		StrokeLine(a,b);
	}
	for(amp = 0.5; amp > minAmplitude; amp /= 2)
	{
		b.y = a.y = amp2y(amp);
		StrokeLine(a,b);
	}
	SetHighColor(192,192,192);
	double freq = 1000.0;
	a.x = b.x = freq2x(freq);
	a.y = top;
	b.y = top+height;
	StrokeLine(a,b);
	double delta = 1000;
	int i = 9;
	for(freq = 1000; freq < maxFrequency; )
	{
		freq += delta;
		if (--i == 0) {
			delta *= 10;
			i = 9;
		}
		a.x = b.x = freq2x(freq);
		StrokeLine(a,b);
	}
	delta = 100;
	for(i = 9, freq = 1000; freq > minFrequency; )
	{
		freq -= delta;
		if (--i == 0) {
			delta /= 10;
			i = 9;
		}
		a.x = b.x = freq2x(freq);
		StrokeLine(a,b);
	}
}

void ResponseGraph::PlotGraph()
{
	float x,a;
	double f;
	BPoint p;
	int i;
	int j;
	p.x = left;
	p.y = phase2y(phases[0]);
	SetHighColor(0,255,0);
	StrokeLine(p,p);
	for(x = 0; x < width; x++)
	{
		a = x * 1024.0 / width;
		i = a;
		f = phases[i];
		p.x = x + left;
		p.y = phase2y(f);
		StrokeLine(p);
	}
	p.x = left;
	p.y = amp2y(amplitudes[0]);
	SetHighColor(0,0,255);
	StrokeLine(p,p);
	for(x = 1; x < width; x++)
	{
		a = x * 1024.0 / width;
		i = a;
		f = amplitudes[i];
		p.x = x + left;
		p.y = amp2y(f);
		StrokeLine(p);
	}
}

void ResponseGraph::UpdateGraph()
{
	double *a = amplitudes;
	double *p = phases;
	double x;
	double min,max;
	min = 0.5;
	max = 2.0;
	for(int i = 0; i < 1024; i++)
	{
		x = i;
		x /= 1024.0;
		x = pow(maxFrequency / minFrequency, x) * minFrequency;
		gFilter->Response(x,a,p);
		if (*a < min) min = *a;
		if (*a > max) max = *a;
		a++;
		p++;
	}
	minAmplitude = min;
	maxAmplitude = max;
	
	Window()->Lock();
	Invalidate();
	Window()->Unlock();
}

double ResponseGraph::x2freq(float x)
{
	double a = maxFrequency / minFrequency;
	double b = (x - left) / width;
	double c = pow(a,b) * (double)minFrequency;
	return c;
}

float ResponseGraph::freq2x(double freq)
{
	double a = freq / minFrequency;
	double b = log(a);
	double c = log(maxFrequency / minFrequency);
	a = b / c;
	a *= width;
	a += left;
	return a;
}

double ResponseGraph::y2amp(float y)
{
	double a = maxAmplitude / minAmplitude;
	double b = (y - top) / height;
	double c = pow(a,b) * minAmplitude;
	return c;
}

float ResponseGraph::amp2y(double amp)
{
	double a = amp / minAmplitude;
	double b = log(a);
	double c = log(maxAmplitude / minAmplitude);
	a = b / c;
	a *= height;
	a = top + height - a;
	return a;
}

float ResponseGraph::phase2y(double phase)
{
	double y = phase + pi;
	y /= 2* pi;
	y *= height;
	y = top + height - y;
	return y;
}
