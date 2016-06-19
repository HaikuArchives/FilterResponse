// FRApp.h

#pragma once

#include <Application.h>
#include "Benaphore.h"

#define SRATE 44100
#define STREAM_BUF_COUNT 4
#define STREAM_BUF_SIZE 512
#define SAMPLE_SIZE 2
#define CHANNEL_COUNT 2
#define SAMPLE_COUNT (STREAM_BUF_SIZE/(SAMPLE_SIZE*CHANNEL_COUNT))
#define SAMPLE_BUF_COUNT STREAM_BUF_COUNT
#define BUF_COUNT 3

class FRPrefs;
class FRDisplayWindow;
class FRNumbersWindow;
class Filter;

extern FRPrefs *gPreferences;

class FRApp : public BApplication
{
public:
	FRApp();
	~FRApp();
	
	virtual	void	MessageReceived(BMessage *msg);
	virtual	void	ReadyToRun();
	virtual bool	QuitRequested();
private:
	FRDisplayWindow *fDisplayWindow;
	FRNumbersWindow *fNumbersWindow;
	
			void	DoNumberChange(int i);
	
	// audio stuff
	BSubscriber *ears;
	BSubscriber *mouth;
	BDACStream *dac;
	BADCStream *adc;
	float srate;
	float *samplesL[BUF_COUNT];
	float *samplesR[BUF_COUNT];
	Benaphore bufaphore[BUF_COUNT];
volatile int read_index, write_index;
	
	long proc_samples;
	bool fBypass;
	BStopWatch *watch1,*watch2;
	/* sample write (adc) */
	/* processing thread */
	thread_id proc_thread;
	bool proc_continue;
	
			void	InitAudio();
			void	StopAudio();
			
	static	bool	_listen(void *data, char *buf, size_t count,
						void *header);
			bool	Listen(int16 *data, int32 count);
	
	static	bool	_talk(void *data, char *buf, size_t count,
						void *header);
			bool	Talk(int16 *data, int32 count);
};

BMessage* NewFilterMessage(int32 filter);
Filter* NewFilter(int32 filter);

const int32 FILTER_MESSAGE = 'filt';
const int32 SHOW_WINDOWS_MESSAGE = 'show';

enum {
	filterCoefficients = 0,
	filterLowPass,
	filterButterBand2a,
	filterButterBand2b,
	kNumFilterKinds
};

extern char *gMenuLabels[];
extern Filter *gFilter;
