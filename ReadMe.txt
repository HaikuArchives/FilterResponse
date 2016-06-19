Filter Response
©1998 Matt Connolly

About Filter Response

This application shows you the frequency response and phase response of a second order IIR filter, sampled at 44.1kHz. This is a very specific application which expects audio in stereo, 16-bit at 44.1kHz and wants to be ble to specify the system's audio buffer configuration.

For the most part, it is an exercise in writing Be applications, (and porting code from MacOS), and also to refresh my memory on digital filtering. Now it has been updated for use with R3 (currently compiled for PowerPC only, someone please compile for Pentium and tell me how it goes!) and has had some very minor improvements.

I'm very keen to hear feed back on this application's performance, effects on the system (if any), and general appearance, etc, etc, etc..... .

This application makes use of the Preferences Shared Library, as posted by me. I am aware that there are other Preferences libraries out there, and hope that one of these will be adopted into the OS. Long live shared code and uniformity between applications!

You may run the application several times, in which case you get several filters. Because of the way that streaming works, you get a very nice effect of being able to hear the filters being added together. Very nice. In practice, I found the system starts to get a bit clunky with about four or more audio apps all running at the same time.

Associated Files

Benaphore.cpp		Source code for an atomically accelerated semaphore, truely a lovely
				piece of work.
Benaphore.h		Its header file.
Filter.cp			Source code for the actual filters. Handles execution and analysis.
Filter.h
FilterResponse		The application - two clicks here will do it every time.
FilterResponse.proj	BeIDE Project file - use this to compile the application.
FilterResponse.settings	Preferences file, this file can be deleted; a new one will be made.
FilterResponse.txt	This file.
FRApp.cp			Source code for the Application object
FRApp.h
FRDisplayWindow.cp	Source code for the Frequency Response Display Window.
FRDisplayWindow.h
FRNumbersWindow.cp	Source code for the filter selection and definition window.
FRNumbersWindow.h
FRPrefs.cp			Source code for our preferences object.
FRPrefs.h
Preferences.cp		Source code for Preferences management, as in my Preferences post.
Preferences.h


Known bugs:

1. When running several copies of this application, clicking occurs in the output stream when doing other tasks, such as changing workspaces, moving windows, etc.

About the Author / Modifier:
	- Matt Connolly -
BeOS, MacOS, Rhapsody programming; Music isn't everything, it's the only thing.
matt@musiclab.com.au
