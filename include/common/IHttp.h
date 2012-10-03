#ifndef __IHTTP_H
#define __IHTTP_H

#include "INet.h"

#define GET_TYPE_NORMAL				0x1 // Use basic URI, else file + host format
#define GET_NO_CACHE				0x2

class IHttp
{
	char *Proxy;
	int ProxyPort;

	int BufferLen;
	char *Buffer;
	
	GAutoPtr<GSocketI> Socket;	// commands
	int ResumeFrom;
	char *FileLocation;
	char *Headers;
	bool NoCache;
	char *AuthUser;
	char *AuthPassword;

public:
	Progress *Meter;

	IHttp();
	virtual ~IHttp();

	void SetResume(int i) { ResumeFrom = i; }
	void SetProxy(char *p, int Port);
	void SetNoCache(bool i) { NoCache = i; }
	void SetAuth(char *User = 0, char *Pass = 0);

	// Data
	GSocketI *Handle() { return Socket; }
	char *AlternateLocation() { return FileLocation; }
	char *GetHeaders() { return Headers; }
	
	// Connection
	bool Open(GAutoPtr<GSocketI> S, char *RemoteHost, int Port = 0);
	bool Close();
	bool IsOpen();
	GSocketI *GetSocket() { return Socket; }

	// File
	bool GetFile(	GViewI *Parent,
					char *File,
					GStream &Out,
					int Format = GET_TYPE_NORMAL,
					int *ProtocolStatus = 0,
					int *Length = 0);

	// General
	bool Get(		char *Uri,
					const char *InHeaders,
					int *ProtocolStatus,
					GStream *Out,
					GStream *OutHeaders = 0);

	bool Post(		char *Uri,
					const char *ContentType,
					GStream *In,
					int *ProtocolStatus,
					GStream *Out,
					GStream *OutHeaders = 0,
					char *InHeaders = 0);
};

#endif
