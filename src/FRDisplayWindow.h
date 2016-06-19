// FRDisplayWindow.h

#pragma once

#include <Window.h>
#include <View.h>

class ResponseGraph : public BView
{
public:
	ResponseGraph(BRect r);
	virtual ~ResponseGraph();
	
	virtual void Draw(BRect area);
	virtual void FrameResized(float x, float y);
	
	void DrawGrid();
	void PlotGraph();
	void UpdateGraph();
	
	double x2freq(float x);
	float freq2x(double freq);
	double y2amp(float y);
	float amp2y(double amp);
	double y2phase(float y);
	float phase2y(double phase);
	
	double minFrequency;
	double maxFrequency;
	double minAmplitude;
	double maxAmplitude;
	float width,height,left,top;
	
	double *amplitudes;
	double *phases;
	double pi;
};


class FRDisplayWindow : public BWindow
{
public:
	FRDisplayWindow(BRect rect);
	virtual void Quit();
	
	virtual void MessageReceived(BMessage *message);
	virtual bool QuitRequested();
	
	void UpdateGraph() { fGraph->UpdateGraph(); }
	
	ResponseGraph *fGraph;
};
