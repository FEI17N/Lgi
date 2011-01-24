#ifndef _OPTION_FILE_H_
#define _OPTION_FILE_H_

#include "GVariant.h"
#include "GXmlTree.h"
#include "GSemaphore.h"

class LgiClass GOptionsFile : public GXmlTag, public GSemaphore
{
public:
	enum PortableType
	{
		UnknownMode,
		PortableMode,
		DesktopMode,
	};

private:
	GAutoString File;
	GAutoString Error;
	bool Dirty;
	
	char *LockFile;
	int LockLine;

	void _Init();
	bool _OnAccess(bool Start);

	// Don't allow non-threadsafe access to these
	char *GetAttr(char *Name) { return GXmlTag::GetAttr(Name); }
	int GetAsInt(char *Name) { return GXmlTag::GetAsInt(Name); }
	bool SetAttr(char *Name, char *Value) { return GXmlTag::SetAttr(Name, Value); }
	bool SetAttr(char *Name, int Value) { return GXmlTag::SetAttr(Name, Value); }
	bool DelAttr(char *Name) { return GXmlTag::DelAttr(Name); }
	void InsertTag(GXmlTag *t) { GXmlTag::InsertTag(t); }
	void RemoveTag() { GXmlTag::RemoveTag(); }
	GXmlTag *GetTag(char *Name, bool Create = false) { return GXmlTag::GetTag(Name, Create); }

protected:
	virtual void _Defaults() {}

public:
	GOptionsFile(char *FileName = 0);
	GOptionsFile(PortableType Mode, char *BaseName = 0);
	~GOptionsFile();

	void SetFile(char *f);
	bool SetMode(PortableType Mode, char *BaseName = 0);

	bool IsValid();
	char *GetFile() { return File; }
	char *GetError() { return Error; }
	bool Serialize(bool Write);
	bool DeleteValue(char *Name);

	bool CreateTag(char *Name);
	bool DeleteTag(char *Name);
	GXmlTag *LockTag(char *Name, char *File, int Line);
};

#endif