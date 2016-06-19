// FRPrefs.cp

#include "FRPrefs.h"
#include "FRApp.h"

FRPrefs::FRPrefs()
	: Preferences('0020',"FilterResponse.settings",kApplicationDirectory)
{
}

void FRPrefs::AddDefaultData()
{
	AddRect("Display Window", BRect( 30.0, 70.0, 350.0, 310.0 ) );
	AddRect("Numbers Window", BRect( 360.0, 70.0, 616.0, 260.0 ) );
	AddInt32("Filter Kind", filterCoefficients);
	AddDouble("Sample Rate", 44100.0);
	AddDouble("Last Frequency", 1000.0);
}

