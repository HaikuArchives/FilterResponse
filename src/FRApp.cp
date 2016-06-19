// FRApp.cp

#include "FRApp.h"
#include "FRPrefs.h"
#include "FRDisplayWindow.h"
#include "FRNumbersWindow.h"
#include "Filter.h"
#include "Benaphore.h"


char *gMenuLabels[] = {
	"Coefficients",
	"Simple Low Pass",
	"Butterworth Band Pass #a",
	"Butterworth Band Pass #b"
};

double gSampleRate;
Filter *gFilter = NULL;
Benaphore gProcessaphore("Processaphore");
FRPrefs *gPreferences;

//
//	main()
//

main()
{
	FRApp app;
	app.Run();
}

//
//	FRApp::FRApp()
//

FRApp::FRApp()
	: BApplication("application/x-vnd.FilterResponse")
{
	gFilter = NULL;
	
	gPreferences = new FRPrefs;
	gPreferences->Open();
	
	BRect rect;
	
	gPreferences->FindRect("Display Window",&rect);
	fDisplayWindow = new FRDisplayWindow(rect);
	gPreferences->FindRect("Numbers Window",&rect);
	fNumbersWindow = new FRNumbersWindow(rect);
	gPreferences->FindDouble("Sample Rate", &gSampleRate);
	
	
	for(int i = 0; i < MAX_FILTER_SIZE; i++)
	{
		Filter::x[0][i] = 0.0;
		Filter::x[1][i] = 0.0;
		Filter::y[0][i] = 0.0;
		Filter::y[1][i] = 0.0;
	}
	
	InitAudio();
}


FRApp::~FRApp()
{
	delete gFilter;
	delete gPreferences;
	gPreferences = NULL;
}

//
//	FRApp::MessageReceived
//

void FRApp::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
	case FILTER_MESSAGE:
		// prevent the windows from doing anything with
		// the filter while we change it
		fDisplayWindow->Lock();
		fNumbersWindow->Lock();
		
		int32 i;
		msg->FindInt32("Filter Kind",&i);
		BMenuItem *menuItem = fNumbersWindow->fMenu->FindItem(gMenuLabels[i]);
		menuItem->SetMarked(true);
		if (gFilter) {
			gFilter->AdjustNumbersWindow(fNumbersWindow,false);
			delete gFilter;
		}
		
		gFilter = NewFilter(i);
		gFilter->AdjustNumbersWindow(fNumbersWindow,true);
		gFilter->CalculateCoefficients();
		
		fNumbersWindow->Unlock();
		fDisplayWindow->Unlock();
		fDisplayWindow->UpdateGraph();
		
		break;
	case SHOW_WINDOWS_MESSAGE:
		fDisplayWindow->Show();
		fNumbersWindow->Show();
		break;
	case msgText:
		i = msg->FindInt16("Index");
		DoNumberChange(i);
		break;
	default:
		BApplication::MessageReceived(msg);
	}
}

void FRApp::ReadyToRun()
{
	int32 filter;
	gPreferences->FindInt32("Filter Kind",&filter);
	PostMessage(NewFilterMessage(filter));
	PostMessage(new BMessage(SHOW_WINDOWS_MESSAGE));
}

bool FRApp::QuitRequested()
{
	StopAudio();
	gPreferences->ReplaceInt32("Filter Kind", gFilter->fFilterKind);
	gPreferences->ReplaceDouble("Sample Rate", gSampleRate);
	fDisplayWindow->Lock();
	fDisplayWindow->Quit();
	fNumbersWindow->Lock();
	fNumbersWindow->Quit();
	return true;
}

void FRApp::DoNumberChange(int index)
{
	char str[256];
	strcpy(str, fNumbersWindow->fText[index]->Text());
	double f;
	sscanf(str,"%lf",&f);
	gFilter->NumberChanged(index, f);
	gFilter->CalculateCoefficients();
	fDisplayWindow->UpdateGraph();
}

BMessage *NewFilterMessage(int32 filter)
{
	BMessage *msg = new BMessage(FILTER_MESSAGE);
	msg->AddInt32("Filter Kind", filter);
	return msg;
}

Filter* NewFilter(int32 filter)
{
	switch(filter)
	{
	case filterCoefficients:
		return new FilterCoefficients(gSampleRate);
	case filterLowPass:
		return new FilterLowPass(gSampleRate);
	case filterButterBand2a:
		return new FilterButterBand2a(gSampleRate);
	case filterButterBand2b:
		return new FilterButterBand2b(gSampleRate);
	default:
		return new Filter(filter, gSampleRate);
	}
}


void FRApp::InitAudio()
{
	for(int i = 0; i < BUF_COUNT; i++) {
		samplesL[i] = (float*) malloc(sizeof(float) * STREAM_BUF_SIZE);
		samplesR[i] = (float*) malloc(sizeof(float) * STREAM_BUF_SIZE);
	}
	
	watch1 = new BStopWatch("Input block time");
	watch1->Suspend();
	watch1->Reset();
	watch2 = new BStopWatch("Output block time");
	watch2->Suspend();
	watch2->Reset();
	
	dac = new BDACStream();
	dac->SetStreamBuffers(STREAM_BUF_SIZE, STREAM_BUF_COUNT);
	dac->EnableDevice(B_ADC_IN, TRUE);
	dac->EnableDevice(B_LOOPBACK, FALSE);
	dac->SetSamplingRate(SRATE);
	
	adc = new BADCStream();
	adc->SetSamplingRate(SRATE);
	adc->SetStreamBuffers(STREAM_BUF_SIZE, STREAM_BUF_COUNT);
	adc->BoostMic(FALSE);

	ears = new BSubscriber("ears");
	mouth = new BSubscriber("mouth");

	ears->Subscribe(adc);
	mouth->Subscribe(dac);
	
	proc_continue = true;
	fBypass = false;
	
	read_index = 1;
	write_index = 0;
	ears->EnterStream(NULL, true, (void *)this, _listen, NULL, true);
	// wait for the first ears to finish, ie. unlock it.
	while(read_index == 1) {
	};
	// now we can start the mouth stream, it should a buffer behind
	mouth->EnterStream(NULL, true, (void *)this, _talk, NULL, true);
}

void FRApp::StopAudio()
{
	proc_continue = false;
	for(int i = 0; i < BUF_COUNT; i++)
		bufaphore[i].Unlock();			// make sure the threads aren't waiting
	mouth->ExitStream();
	ears->ExitStream();
	
	delete ears;
	delete mouth;
	delete adc;
	delete dac;
	delete watch1;
	delete watch2;
	
	for(int i = 0; i < BUF_COUNT; i++) {
		free(samplesL[i]);
		free(samplesR[i]);
	}
}

bool FRApp::_listen(void *data, char *buf, size_t count, void *header)
{
	return ((FRApp*)data)->Listen((int16 *)buf, count);
}

bool FRApp::Listen(int16 *buffer, int32 bytes)
{
	watch1->Resume();
	status_t err = bufaphore[read_index].Lock();
	watch1->Suspend();
	if (err != B_NO_ERROR)
		return false;
	
	if (!proc_continue) return false;
	
	// translate the buffer into floats
	int16 *in = buffer - 1;
	register float f;
	float *outL = samplesL[read_index] - 1;
	float *outR = samplesR[read_index] - 1;
	int count = bytes / (SAMPLE_SIZE * CHANNEL_COUNT);
	proc_samples = count;
	while(--count >= 0) {
		f = *++in;
		f /= 32768.0;
		*++outL = f;
		f = *++in;
		f /= 32768.0;
		*++outR = f;
	}
	
	float *inputs[2] = { samplesL[read_index], samplesR[read_index] };
	float *outputs[2] = { samplesL[read_index], samplesR[read_index] };
	
	// apply the filter
	if (gFilter && !fBypass) {
		if (gProcessaphore.Lock() == B_NO_ERROR) {
			gFilter->Process(inputs, outputs, proc_samples);
			gProcessaphore.Unlock();
		}
	}
	
	// release the buffer benaphore
	bufaphore[read_index].Unlock();
	
	// increment our index.
	read_index = (read_index + 1) % BUF_COUNT;
	
	return true;
}

bool FRApp::_talk(void *data, char *buf, size_t count, void *header)
{
	return ((FRApp*)data)->Talk((int16 *)buf, count);
}

bool FRApp::Talk(int16 *buffer, int32 bytes)
{
	// acquire talk semaphore
	watch2->Resume();
	status_t err = bufaphore[write_index].Lock();
	watch2->Suspend();
	if (err != B_NO_ERROR)
		return false;
	
	if (!proc_continue) return false;
	
	float *inL = samplesL[write_index] - 1;
	float *inR = samplesR[write_index] - 1;
	int16 *buf = buffer - 1;
	register float f1,f2;
	int32 left,right;
	
	// copy to the output buffer
	int count = bytes / (SAMPLE_SIZE * CHANNEL_COUNT);
	if (count != proc_samples)
		debugger("Something's not rite!\n");
	while (--count >= 0)
	{
		left = buf[1];
		right = buf[2];
		f1 = *++inL;
		f2 = *++inR;
		left += (int32) (f1 * 32767.0);
		right += (int32) (f2 * 32767.0);
		if (left < -32768) left = -32768;
		else if (left > 32767) left = 32767;
		if (right < -32768) right = -32768;
		else if (right > 32767) right = 32767;
		*++buf = left;
		*++buf = right;
	}
	
	// release the input benaphore
	bufaphore[write_index].Unlock();
	
	// increment our index;
	write_index = (write_index + 1) % BUF_COUNT;
	
	return true;
}

