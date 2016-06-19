// Preferences.h
//
// ©1998 Matt Connolly
//
// See Preferences.txt for more info.
//

#ifndef _Preferences_H_
#define _Preferences_H_

// use one of these to indicate where the preferences file will be.

enum {
	kSettingsDirectory,
	kApplicationDirectory,
	kSpecificDirectory
};

// strings to identify the signature and version data members

#define kSignatureString	"Preferences_signature"
#define kVersionString		"Preferences_version"

//
// class Preferences
//

class Preferences
{
public:
		Preferences(int32 version, const char *pathname, int32 where);
		~Preferences();
		
		status_t	Open();		// open preferences file
		status_t	Close();	// close the preferences file
		
protected:
		// sub-classes may override if appropriate.

virtual	void		AddDefaultData();
virtual	status_t	OpenCreate();	// called to create the new file
virtual	status_t	Update();		// update to the current version
virtual	status_t	SetFileInfo(BNodeInfo *info);
	
		// sub-classes to access private data.
		void		Changed() { fChanged = true; }
		void		SetVersion(int32 v) { fVersion = v; }
		int32		GetVersion() { return fVersion; }
		
public:

		// we're almost like a BMessage, aren't we?
				
		status_t	AddRect(const char *name, BRect a_rect);
		status_t	AddPoint(const char *name, BPoint a_point);
		status_t	AddString(const char *name, const char *a_string);
		status_t	AddInt8(const char *name, int8 val);
		status_t	AddInt16(const char *name, int16 val);
		status_t	AddInt32(const char *name, int32 val);
		status_t	AddInt64(const char *name, int64 val);
		status_t	AddBool(const char *name, bool a_boolean);
		status_t	AddFloat(const char *name, float a_float);
		status_t	AddDouble(const char *name, double a_double);
		status_t	AddRef(const char *name, const entry_ref *ref);
		status_t	AddData(const char *name,
							type_code type,
							const void *data,
							ssize_t numBytes,
							bool is_fixed_size = TRUE,
							int32 num_adds = 1);

		status_t	FindRect(const char *name, BRect *rect) const;
		status_t	FindRect(const char *name, int32 index, BRect *rect) const;
		status_t	FindPoint(const char *name, BPoint *pt) const;
		status_t	FindPoint(const char *name, int32 index, BPoint *pt) const;
		status_t	FindString(const char *name, const char **str) const;
		status_t	FindString(const char *name, int32 index, const char **str) const;
		status_t	FindInt8(const char *name, int8 *value) const;
		status_t	FindInt8(const char *name, int32 index, int8 *val) const;
		status_t	FindInt16(const char *name, int16 *value) const;
		status_t	FindInt16(const char *name, int32 index, int16 *val) const;
		status_t	FindInt32(const char *name, int32 *value) const;
		status_t	FindInt32(const char *name, int32 index, int32 *val) const;
		status_t	FindInt64(const char *name, int64 *value) const;
		status_t	FindInt64(const char *name, int32 index, int64 *val) const;
		status_t	FindBool(const char *name, bool *value) const;
		status_t	FindBool(const char *name, int32 index, bool *value) const;
		status_t	FindFloat(const char *name, float *f) const;
		status_t	FindFloat(const char *name, int32 index, float *f) const;
		status_t	FindDouble(const char *name, double *d) const;
		status_t	FindDouble(const char *name, int32 index, double *d) const;
		status_t	FindRef(const char *name, entry_ref *ref) const;
		status_t	FindRef(const char *name, int32 index, entry_ref *ref) const;
		status_t	FindData(const char *name,
							type_code type,
							const void **data,
							ssize_t *numBytes) const;
		status_t	FindData(const char *name,
							type_code type,
							int32 index,
							const void **data,
							ssize_t *numBytes) const;
		status_t	ReplaceRect(const char *name, BRect a_rect);
		status_t	ReplaceRect(const char *name, int32 index, BRect a_rect);
		status_t	ReplacePoint(const char *name, BPoint a_point);
		status_t	ReplacePoint(const char *name, int32 index, BPoint a_point);
		status_t	ReplaceString(const char *name, const char *a_string);
		status_t	ReplaceString(	const char *name,
									int32 index, 
									const char *a_string);
		status_t	ReplaceInt8(const char *name, int8 val);
		status_t	ReplaceInt8(const char *name, int32 index, int8 val);
		status_t	ReplaceInt16(const char *name, int16 val);
		status_t	ReplaceInt16(const char *name, int32 index, int16 val);
		status_t	ReplaceInt32(const char *name, int32 val);
		status_t	ReplaceInt32(const char *name, int32 index, int32 val);
		status_t	ReplaceInt64(const char *name, int64 val);
		status_t	ReplaceInt64(const char *name, int32 index, int64 val);
		status_t	ReplaceBool(const char *name, bool a_bool);
		status_t	ReplaceBool(const char *name, int32 index, bool a_bool);
		status_t	ReplaceFloat(const char *name, float a_float);
		status_t	ReplaceFloat(const char *name, int32 index, float a_float);
		status_t	ReplaceDouble(const char *name, double a_double);
		status_t	ReplaceDouble(	const char *name,
									int32 index,
									double a_double);
		status_t	ReplaceRef(	const char *name,
								const entry_ref *ref);
		status_t	ReplaceRef(	const char *name,
								int32 index,
								const entry_ref *ref);
		status_t	ReplaceData(const char *name,
								type_code type,
								const void *data,
								ssize_t data_size);
		status_t	ReplaceData(const char *name,
								type_code type,
								int32 index,
								const void *data,
								ssize_t data_size);


protected:
	bool fChanged;
	bool fOpen;
	
private:
		status_t	Write();	// write the entire preferences
		status_t	Read();		// read the entire preferences
	
	BEntry *fEntry;
	BFile *fFile;
	BMessage *fMessage;
	int32 fVersion;
	char fSignature[B_MIME_TYPE_LENGTH];
};

#endif
