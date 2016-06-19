// FRNumbersWindow.h

#pragma once

class BTextControl;
class BPopUpMenu;
class BMenuField;

class FRNumbersWindow : public BWindow
{
public:
	FRNumbersWindow(BRect r);
	
	virtual void Quit();
	virtual void MessageReceived(BMessage *message);
	
	BMenuField *fPopUp;
	BPopUpMenu *fMenu;
	BTextControl *fText[5];
};

const int32 msgText = 'mtxc';
