// FRNumbersWindow.cp

#include "FRNumbersWindow.h"
#include "FRPrefs.h"
#include "FRApp.h"

const BRect rp1(20, 10, 240, 30);
const BRect rt(20, 40, 240, 60);
const float rt_delta = 30;

FRNumbersWindow::FRNumbersWindow(BRect r)
	: BWindow(r, "Filter", B_TITLED_WINDOW, B_NOT_CLOSABLE|B_NOT_ZOOMABLE)
{
	Lock();
	
	fMenu = new BPopUpMenu("Filter kind");
	BMenuItem *item;
	for(int i = 0; i < kNumFilterKinds; i++) {
		item = new BMenuItem(
				gMenuLabels[i],
				NewFilterMessage(i));
		item->SetTarget(be_app);
		fMenu->AddItem(item);
	}
	fMenu->SetRadioMode(true);
	
	fPopUp = new BMenuField(rp1, "filter kind", "Filter Kind:", fMenu);
	fPopUp->SetDivider(80.0);
	AddChild(fPopUp);
	
	r = rt;
	int i;
	for(i = 0; i < 5; i++)
	{
		BMessage *msg = new BMessage(msgText);
		msg->AddInt16("Index",i);
		char name[256];
		sprintf(name,"text%i",i);
		fText[i] = new BTextControl(r, name, "", "", msg);
		fText[i]->SetTarget(be_app);
		fText[i]->SetDivider(80);
		AddChild(fText[i]);
		fText[i]->Hide();
		r.top += rt_delta;
		r.bottom += rt_delta;
	}
	
	Unlock();
}

void FRNumbersWindow::Quit()
{
	BRect r = Frame();
	gPreferences->ReplaceRect("Numbers Window", r);
	
	BWindow::Quit();
}

void FRNumbersWindow::MessageReceived(BMessage *message)
{
	switch (message->what)
	{
	default:
		BWindow::MessageReceived(message);
	}
}

