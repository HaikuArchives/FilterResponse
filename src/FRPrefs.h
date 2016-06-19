// FRPrefs.h

#pragma once

#include "Preferences.h"

class FRPrefs : public Preferences
{
public:
	FRPrefs();
	virtual void AddDefaultData();
};

extern FRPrefs *gPreferences;
