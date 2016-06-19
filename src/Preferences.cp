// Preferences.cp
//
// must have a BApplication object.

#include "Preferences.h"
#include <Message.h>

#define returnBad(x) if ((x) != B_OK) return x
#define exitBad(x) if ((x) != B_OK) return

#pragma export on

//
// Preferences::Preferences
//
// constructor: initialise the class, etc. Find out the information about
// the current application, use it if necessary. Create a BEntry object
// to use as a reference for the preference file and initialise it.
//

Preferences::Preferences(int32 version, const char *pathname, int32 where)
{
	// initialise our variables
	fChanged = false;
	fOpen = false;
	fFile = new BFile;
	fMessage = new BMessage('Pref');
	fVersion = version;
	
	// get info about the app
	app_info info; 
	if( be_app->GetAppInfo(&info) != B_OK ) return;
	strcpy(fSignature, info.signature);
	
	// create the BEntry
	fEntry = new BEntry;
	
	// initialise it to the specified location & name
	switch (where)
	{
	case kSettingsDirectory:		// prefs in '/boot/home/config/settings/...'
		char name[256];
		strcpy(name,"/boot/home/config/settings/");
		strcat(name,pathname);
		fEntry->SetTo(name);
		break;
	case kApplicationDirectory:		// prefs in the same directory as the app - my favourite
		entry_ref ref = info.ref;
		ref.set_name(pathname);
		fEntry->SetTo(&ref);
		break;
	case kSpecificDirectory:		// prefs in a specific location if you want
		fEntry->SetTo(pathname);
		break;
	}
}

//
// Preferences::~Preferences
//
// destructor: ensure the file is updated, closed, and free all the objects created
// by this one.
//

Preferences::~Preferences()
{
	// make sure the file is closed
	Close();
	
	// free up our objects
	delete fFile;
	delete fEntry;
	delete fMessage;
}

//
// void Preferences::AddDefaultData()
//
// Never call directly; called by 'OpenCreate'
//
// Does nothing. This is for sub classes to over-ride if necessary to
// create a default set of data should there be no preferences file.
//

void Preferences::AddDefaultData()
{
	// does nothing - applications create a subclass and implement this.
	// use BMessage's AddData and related functions to atore the data,
	// and then use ReplaceData from the Application when something changes
}

//
// status_t Preferences::Open()
//
// Call this function after constructing the object to open the file.
//
// Open the file and read it. If no file exists, or the file doesn't match
// what we expect, make a new one from the defaults.
//

status_t Preferences::Open()
{
	status_t err = B_OK;
	// do nothing if we're already open
	if (!fOpen)
	{
		// open the file
		err = fFile->SetTo(fEntry,B_READ_WRITE);
		if (err == B_ENTRY_NOT_FOUND)
		{
			// if it wasn't found, make a new one
			return OpenCreate();
		}
		else	// otherwise, some error we can't deal with
			if (err != B_OK) {
				// add default data so the program will always find something
				AddDefaultData();
				return err;
			}
		
		fOpen = true;
		// read the contents of the file
		err = Read();
		if (err != B_OK)
		{
			// if we couldn't read it properly, create a new one
			fOpen = false;
			return OpenCreate();
		}
		else
		{
			// check the signature and version, throw away the file if from another app.
			char *signature;
			int32 version;
			
			if (FindString(kSignatureString,&signature) != B_OK)
				return OpenCreate();
			if (strcmp(fSignature, signature) != 0)
				return OpenCreate();
			
			if (FindInt32(kVersionString, &version) != B_OK)
				return OpenCreate();	// no version number - make a new one!
			if (version < fVersion)		// for an older version file,
				return Update();		// update it.
			if (version > fVersion)		// for a newer one,
				return OpenCreate();	// kick it out and make a new one.
			
			// if we get to this point, everything must have been ok.
		}
	}
	return err;
}

//
// status_t Preferences::OpenCreate()
//
// Never call directly.
//
// Create a new preferences file. This function is virtual so that you can have a
// totally different method of creating the file should you need to.
// 

status_t Preferences::OpenCreate()
{
	// clear the message
	delete fMessage;
	fMessage = new BMessage('Pref');
	// add the signature and version number
	AddString(kSignatureString, fSignature);
	AddInt32(kVersionString, fVersion);
	// add the default data
	AddDefaultData();
	// create the file
	status_t err;
	err = fFile->SetTo(fEntry, B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);
	if (err != B_OK)
		return err;
	
	// set the file info
	BNodeInfo *info = new BNodeInfo(fFile);
	SetFileInfo(info);
	delete info;
	
	// write the preferences to the file
	fOpen = true;
	return Write();
}

//
// status_t Preferences::Update()
//
// Never call directly; called by 'Open'
//
// This function is called when a preferences file is found that is an older version
// of the expected. A new set of preferences data is created, and where some data
// is found in both, the older version is kept. This way when the program grows,
// users with old preferences who go to the new version can still use their old
// settings if appropriate. This will polute the file with unnecesarry data if the
// new version doesn't need those things, but that shouldn't be much of a problem.
//
// you may override this to convert the old file more intelligently, if requried.
//

status_t Preferences::Update()
{
	status_t err;
	// update the version number
	err = fMessage->ReplaceInt32(kVersionString, fVersion);
	if (err != B_OK)
		return err;
	// create a new BMessage with the default data
	BMessage *temp = new BMessage('Pref');
	BMessage *swap = fMessage;
	fMessage = temp;
	AddDefaultData();
	fMessage = swap;
	
	// we will compare every name in the new data message
	long counter = temp->CountNames(B_ANY_TYPE);
	long i;
	for(i = 0; i < counter; ++i)
	{
		// get info about a piece of data from the new defaults
		type_code type;
		char *name;
		int32 countFound;
		err = temp->GetInfo(B_ANY_TYPE, i, &name, &type, &countFound);
		if (err != B_OK)
			return err;
		
		// find that name and type in the old version
		int32 countFound2;
		err = fMessage->GetInfo(name, &type, &countFound2);
		
		if (err == B_NAME_NOT_FOUND)	// if we can't find that name & type, copy it
		{
			// get that piece of data in the new default message
			char *dataPtr;
			ssize_t dataSize;
			err = temp->FindData(name, type, &dataPtr, &dataSize);
			if (err != B_OK)
				return err;
			
			// copy it to the old version
			err = fMessage->AddData(name, type, dataPtr, dataSize);
			if (err != B_OK)
				return err;
		}
	}
	// delete the temporary data message
	delete temp;
}

//
// status_t Preferences::SetFileInfo(BNodeInfo *info)
//
// Never call directly; called by 'OpenCreate'
//
// called when a preferences file is created to set the file type, etc. you
// can override this to set any other attributes about the file
//

status_t Preferences::SetFileInfo(BNodeInfo *info)
{
	status_t err = info->SetType("application/preferences");
	if (err == B_OK) {
		err = info->SetPreferredApp(fSignature);
	}
	return err;
}

//
// status_t Preferences::Close()
//
// You may call this to close the preferences file. It will be called by
// the destructor when this object is deleted.
//

status_t Preferences::Close()
{
	if (fOpen)		// only if we're open
	{
		status_t err = B_OK;
		// if we have changed our data,
		if (fChanged)
		{
			// write the entire file again
			err = Write();
		}
		// close the file
		fFile->Unset();
		fOpen = false;
		return err;
	}
	return B_NO_INIT;
}

#pragma export reset

//
// status_t Preferences::Write()
//
// Never call directly.
//
// Writes the contents of the data message to the file as flattened data. As far as I
// know, this should not have any endian related problems.
//

status_t Preferences::Write()
{
	status_t err;
	
	if (fOpen)		// only if we're open...
	{
		// go to the beginning of the file
		err = fFile->Seek(0, SEEK_SET);
		if (err != B_OK)
			return err;
		// write the flattened BMessage to the stream
		err = fMessage->Flatten(fFile);
		if (err != B_OK)
			return err;
		// no we aren't changed any more.
		fChanged = false;
		return B_OK;
	}
	return B_NO_INIT;
}

//
// status_t Preferences::Read()
//
// Never call directly.
//
// Called by Open to read the BMessage containing the preferences data.
//

status_t Preferences::Read()
{
	status_t err = B_NO_INIT;
	
	if (fOpen)		// only if we're open...
	{
		// go to the beginning of the file
		err = fFile->Seek(0, SEEK_SET);
		if (err == B_OK)	// read the BMessage from the file
			err = fMessage->Unflatten(fFile);
	}
	return err;
}


// what we have here is very basic hooks to the BMessage calls. Originally,
// I hadn't intended to use these functions, but I needed a way to tell the
// object that the data in the BMessage had changed. I couldn't figure out
// any other way to do it. If you find out a better way which uses less code,
// please let me know.

#pragma export on

status_t Preferences::AddRect(const char *name, BRect a_rect)
{
	status_t err = fMessage->AddRect(name,a_rect);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddPoint(const char *name, BPoint a_point)
{
	status_t err = fMessage->AddPoint(name,a_point);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddString(const char *name, const char *a_string)
{
	status_t err = fMessage->AddString(name,a_string);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddInt8(const char *name, int8 val)
{
	status_t err = fMessage->AddInt8(name,val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddInt16(const char *name, int16 val)
{
	status_t err = fMessage->AddInt16(name,val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddInt32(const char *name, int32 val)
{
	status_t err = fMessage->AddInt32(name,val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddInt64(const char *name, int64 val)
{
	status_t err = fMessage->AddInt64(name,val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddBool(const char *name, bool a_boolean)
{
	status_t err = fMessage->AddBool(name,a_boolean);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddFloat(const char *name, float a_float)
{
	status_t err = fMessage->AddFloat(name,a_float);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddDouble(const char *name, double a_double)
{
	status_t err = fMessage->AddDouble(name,a_double);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddRef(const char *name, const entry_ref *ref)
{
	status_t err = fMessage->AddRef(name,ref);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::AddData(const char *name,
				type_code type,
				const void *data,
				ssize_t numBytes,
				bool is_fixed_size,
				int32 num_adds)
{
	status_t err = fMessage->AddData(name,type,data,numBytes,is_fixed_size,num_adds);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::FindRect(const char *name, BRect *rect) const
{
	return fMessage->FindRect(name, rect);
}

status_t Preferences::FindRect(const char *name, int32 index, BRect *rect) const
{
	return fMessage->FindRect(name, index, rect);
}

status_t Preferences::FindPoint(const char *name, BPoint *pt) const
{
	return fMessage->FindPoint(name, pt);
}

status_t Preferences::FindPoint(const char *name, int32 index, BPoint *pt) const
{
	return fMessage->FindPoint(name, index, pt);
}

status_t Preferences::FindString(const char *name, const char **str) const
{
	return fMessage->FindString(name, str);
}

status_t Preferences::FindString(const char *name, int32 index, const char **str) const
{
	return fMessage->FindString(name, index, str);
}

status_t Preferences::FindInt8(const char *name, int8 *value) const
{
	return fMessage->FindInt8(name, value);
}

status_t Preferences::FindInt8(const char *name, int32 index, int8 *val) const
{
	return fMessage->FindInt8(name, index, val);
}

status_t Preferences::FindInt16(const char *name, int16 *value) const
{
	return fMessage->FindInt16(name, value);
}

status_t Preferences::FindInt16(const char *name, int32 index, int16 *val) const
{
	return fMessage->FindInt16(name, index, val);
}

status_t Preferences::FindInt32(const char *name, int32 *value) const
{
	return fMessage->FindInt32(name, value);
}

status_t Preferences::FindInt32(const char *name, int32 index, int32 *value) const
{
	return fMessage->FindInt32(name, index, value);
}

status_t Preferences::FindInt64(const char *name, int64 *value) const
{
	return fMessage->FindInt64(name, value);
}

status_t Preferences::FindInt64(const char *name, int32 index, int64 *value) const
{
	return fMessage->FindInt64(name, index, value);
}

status_t Preferences::FindBool(const char *name, bool *value) const
{
	return fMessage->FindBool(name, value);
}

status_t Preferences::FindBool(const char *name, int32 index, bool *value) const
{
	return fMessage->FindBool(name, index, value);
}

status_t Preferences::FindFloat(const char *name, float *f) const
{
	return fMessage->FindFloat(name, f);
}

status_t Preferences::FindFloat(const char *name, int32 index, float *f) const
{
	return fMessage->FindFloat(name, index, f);
}

status_t Preferences::FindDouble(const char *name, double *d) const
{
	return fMessage->FindDouble(name, d);
}

status_t Preferences::FindDouble(const char *name, int32 index, double *d) const
{
	return fMessage->FindDouble(name, index, d);
}

status_t Preferences::FindRef(const char *name, entry_ref *ref) const
{
	return fMessage->FindRef(name, ref);
}

status_t Preferences::FindRef(const char *name, int32 index, entry_ref *ref) const
{
	return fMessage->FindRef(name, index, ref);
}

status_t Preferences::FindData(const char *name,
				type_code type,
				const void **data,
				ssize_t *numBytes) const
{
	return fMessage->FindData(name, type, data, numBytes);
}

status_t Preferences::FindData(const char *name,
				type_code type,
				int32 index,
				const void **data,
				ssize_t *numBytes) const
{
	return fMessage->FindData(name, type, index, data, numBytes);
}

status_t Preferences::ReplaceRect(const char *name, BRect a_rect)
{
	status_t err = fMessage->ReplaceRect(name, a_rect);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceRect(const char *name, int32 index, BRect a_rect)
{
	status_t err = fMessage->ReplaceRect(name, index, a_rect);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplacePoint(const char *name, BPoint a_point)
{
	status_t err = fMessage->ReplacePoint(name, a_point);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplacePoint(const char *name, int32 index, BPoint a_point)
{
	status_t err = fMessage->ReplacePoint(name, index, a_point);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceString(const char *name, const char *a_string)
{
	status_t err = fMessage->ReplaceString(name, a_string);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceString(	const char *name,
						int32 index, 
						const char *a_string)
{
	status_t err = fMessage->ReplaceString(name, index, a_string);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceInt8(const char *name, int8 val)
{
	status_t err = fMessage->ReplaceInt8(name, val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceInt8(const char *name, int32 index, int8 val)
{
	status_t err = fMessage->ReplaceInt8(name, index, val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceInt16(const char *name, int16 val)
{
	status_t err = fMessage->ReplaceInt16(name, val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceInt16(const char *name, int32 index, int16 val)
{
	status_t err = fMessage->ReplaceInt16(name, index, val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceInt32(const char *name, int32 val)
{
	status_t err = fMessage->ReplaceInt32(name, val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceInt32(const char *name, int32 index, int32 val)
{
	status_t err = fMessage->ReplaceInt32(name, index, val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceInt64(const char *name, int64 val)
{
	status_t err = fMessage->ReplaceInt64(name, val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceInt64(const char *name, int32 index, int64 val)
{
	status_t err = fMessage->ReplaceInt64(name, index, val);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceBool(const char *name, bool a_bool)
{
	status_t err = fMessage->ReplaceBool(name, a_bool);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceBool(const char *name, int32 index, bool a_bool)
{
	status_t err = fMessage->ReplaceBool(name, index, a_bool);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceFloat(const char *name, float a_float)
{
	status_t err = fMessage->ReplaceFloat(name, a_float);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceFloat(const char *name, int32 index, float a_float)
{
	status_t err = fMessage->ReplaceFloat(name, index, a_float);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceDouble(const char *name, double a_double)
{
	status_t err = fMessage->ReplaceDouble(name, a_double);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceDouble(	const char *name,
						int32 index,
						double a_double)
{
	status_t err = fMessage->ReplaceDouble(name, index, a_double);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceRef(	const char *name,
					const entry_ref *ref)
{
	status_t err = fMessage->ReplaceRef(name, ref);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceRef(	const char *name,
					int32 index,
					const entry_ref *ref)
{
	status_t err = fMessage->ReplaceRef(name, index, ref);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceData(const char *name,
					type_code type,
					const void *data,
					ssize_t data_size)
{
	status_t err = fMessage->ReplaceData(name, type, data, data_size);
	if (err == B_OK) Changed();
	return err;
}

status_t Preferences::ReplaceData(const char *name,
					type_code type,
					int32 index,
					const void *data,
					ssize_t data_size)
{
	status_t err = fMessage->ReplaceData(name, type, index, data, data_size);
	if (err == B_OK) Changed();
	return err;
}

#pragma export reset
