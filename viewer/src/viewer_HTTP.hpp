/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include "viewer_base.pch.h"

#define HTTP_AGENT "ColladaDOM 3 Reference Viewer"
		   
//urlencode
static std::vector<char> URI_to_ASCII_buf;
static char *URI_to_ASCII(COLLADA::RT::Name URI)
{
	URI_to_ASCII_buf.clear();
	std::vector<char> &out = URI_to_ASCII_buf;	
	for(size_t i=0;i<URI.extent;i++) 
	{
		out.push_back(URI[i]);
		if(out.back()>0) continue;		
		char hex[3]; 
		snprintf(hex,3,"%02x",(unsigned char)out.back()); //itoa
		out.back() = '%'; 
		out.push_back(hex[0]); out.push_back(hex[1]);
	}
	out.push_back('\0'); return &out[0];
}

struct HTTP_agent
{
	//This is pass-through for daeIO::readIn().
	COLLADA::daeOK readIn(void *in, size_t chars);

	inline size_t peekIn(void*,size_t size);

	inline size_t transferred(){ return statistics[0]; }

	//ZAE: Content-Length isn't necessarily the file size.
	inline size_t ContentLength(){ return statistics[1]; }
	inline size_t *ContentRange(){ return content_range; }
	
	static bool _no_progress(const HTTP_agent*){ return true; }
	HTTP_agent(const COLLADA::daeIORequest*, daeIO::Range *rngI, bool progress(const HTTP_agent*)=_no_progress); 
	HTTP_agent():socket(-1){}
	~HTTP_agent();

	//EXPERIMENTAL ZAE-SUPPORT
	size_t content_range[3]; 
	
	bool (*progress)(const HTTP_agent*); void *progress_dialog;	

	//These had belonged to a downloading thread.
	//pulse is a running time. Bs is for bytes/s.
	//statistics[0] is bytes downloaded. And [1]
	//is the total size downloaded, which cannot
	//change under daeIO model.
	COLLADA::RT::Float pulse; int pulse_Bs, statistics[4]; 

	int socket; inline bool connected()
	{
		return socket!=-1; 
		
		#ifdef _WIN32
		daeCTC<-1==SOCKET(~0)>();
		#endif
	}

	COLLADA::daeURI urlencode; COLLADA::daeIO *cache;

private: //INTERNAL

	char resume_RFC1123[30/*INTERNET_RFC1123_BUFSIZE*/];

	void resume_connect(int=-1),resume_disconnect();

	inline bool resume()
	{
		resume_disconnect(); resume_connect(); return connected(); 
	}	
};

void HTTP_agent::resume_disconnect()
{
	if(connected()) 
	{
		#ifdef _WIN32
		closesocket(socket); WSACleanup();
		#else
		close(socket);
		#endif
		socket = -1;
	}
}	
void HTTP_agent::resume_connect(int bytes)
{	
	char bytestr[33] = ""; if(bytes>=0)
	{
		if(bytes!=0) bytes-=1; //Inclusive. Must download at least 1?
		itoa(content_range[0]+bytes,bytestr,10);
	}

	if('\0'==resume_RFC1123[0]) //if(0==transferred())
	{
		resume_RFC1123[29] = 1; //...
		#ifndef _WIN32
		time_t t; time(&t); strftime(resume_RFC1123,30,"%a, %d %b %Y %T GMT",gmtime(&t));
		#else
		SYSTEMTIME st; GetSystemTime(&st); //#ifdef _WIN32
		GetDateFormatA(LOCALE_INVARIANT,0,&st,"ddd, dd MMM yyyy",resume_RFC1123,17);
		GetTimeFormatA(LOCALE_INVARIANT,0,&st," HH':'mm':'ss 'GMT'",resume_RFC1123+16,14);		
		#endif
		assert(resume_RFC1123[29]=='\0'); //Is it 10,000 AD?
	}
	char headers[1024] = "", head[4096];
	snprintf(headers,sizeof(headers),
	"If-Unmodified-Since: %s\r\nRange: bytes=%d-%s\r\n"
	,resume_RFC1123,content_range[0]+statistics[0],bytestr);	
					
	#ifdef _WIN32
	static WSADATA data; WSAStartup(0x202,&data);	
	#endif
	
	//This is the old (deprecated) way.
	//It works, whereas getaddrinfo is not.
	//hostent *he = gethostbyname(host.view);	
	//if(he) sa.sin_addr = *(IN_ADDR*)he->h_addr;
	addrinfo *ai = nullptr;	

	//HACK: 0-TERMINATING
	daeRefView host = urlencode.getURI_host();
	const daeStringCP host_d = *host.end();
	const daeStringCP *script = host_d=='\0'?"":host.end()+1;		
	*(daeStringCP*)host.end() = '\0';
	{
		getaddrinfo(host.view,"http",nullptr,&ai);
		snprintf(head,sizeof(head),"GET /%s HTTP/1.1\r\nHost: %s\r\n%s"
		"User-Agent: " HTTP_AGENT "\r\n"/**/"\r\n",script,host.view,headers);
	}	
	//REPAIRING 0-TERMINATION
	*(daeStringCP*)host.end() = host_d;

	if(ai!=nullptr)
	{
		//Too many #includes.
		socket = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); 
	}
	if(ai!=nullptr&&0>=connect(socket,ai->ai_addr,ai->ai_addrlen))
	{
		int paranoia = strlen(head);
		int sent = send(socket,head,paranoia,0);
		assert(sent==paranoia);
	}
	else resume_disconnect(); freeaddrinfo(ai);
}

/*-Wunused-function
static void HTTP_legalize(char *inout)
{
	for(int i=0;inout[i]!='\0';i++) switch(inout[i])
	{
	//Not allowed by file systems (so says docs)
	case '<': case '>': case ':': case '"': case '/': case '\\': case '|':

	//Not allowed by Explorer.exe (so says Explorer->rename)
	case '*': case '?':					 
					
		inout[i] = '-';	break;

	case ' ': //preference

		inout[i] = '$';	break;
	}
}*/

HTTP_agent::HTTP_agent(const daeIORequest *IO, daeIO::Range *rngI, bool cb(const HTTP_agent*))
{	
	//HACK! Don't use memset/placement-new.
	memset(this,0x00,sizeof(*this)); progress = cb;
	new(&urlencode) daeURI(URI_to_ASCII(IO->remoteURI->getURI_upto<'#'>())); 
	if(rngI!=nullptr&&0!=rngI->size())
	{
		content_range[0] = rngI->first;
		content_range[1] = rngI->second-1;
	}

	//Resolving will replace the percent-encodings with friendly UTF8.
	urlencode.setIsResolved(); assert(IO->remoteURI->getIsResolved()); 
	
	/*TODO: Set up cache.
	wchar_t out[MAX_PATH+1] = L"\\connection";	
	size_t cat = wcslen(Somfolder(out)); out[cat++] = '\\';
	wcscpy_s(out+cat,MAX_PATH-cat,inout); HTTP_legalize(out+cat);
	DeleteFileW(out); wcscat_s(out,L".incomplete"); //cache
	
	HANDLE test = CreateFileA(out,
	GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if(test==INVALID_HANDLE_VALUE)
	{			
		assert(0); return false;
	}		
	else CloseHandle(test);*/
	
	int http_status = 0;

	resume_connect(rngI==nullptr?-1:(int)rngI->size());
	//in->thread = CreateThread
	//(0,0,(LPTHREAD_START_ROUTINE)HTTP_threadmain,in,0,&in->id);
	{	
		//if(in->WinSock==in->method) //NEW METHOD
		{
			char buf[1024];
	
			//Peeking at Content-Length
			//Want to avoid holding onto a buffer, but don't
			//know if this is guaranteed to have enough of a
			//header to say what's what.
			int lim = sizeof(buf)-1;
			int len = recv(socket,buf,lim,MSG_PEEK);
			if(len<0) len = 0; buf[len] = '\0';
			const char *st = strchr(buf,' ');
			if(st!=nullptr) http_status = atoi(st+1);
			const char tok[] = "\r\nContent-Length: ";
			char *p = strstr(buf,tok);
			if(p==nullptr) 
			{
				daeEH::Error<<"Content-Length is not in the HTTP header. Sorry."
				"\n(daeIO needs to know the size. Please request a workaround.)";
			}
			else switch(http_status) //HACK this is holding the download size.
			{
			case 200: case 206: case 412: //Expecting 206 (Partial-Content)
			{	
				statistics[1] = atoi(p+sizeof(tok)-1);				

				//NEW: Content-Length isn't necessarily the file's size.
				char *p = strstr(buf,"\r\nContent-Range: ");
				if(p!=nullptr)
				{
					while(*p!='\0'&&!isdigit(*p)) *p++;
					content_range[0] = atoi(p); 
					while(*p!='\0'&&*p++!='-');
					content_range[1] = std::max<int>(atoi(p),content_range[0]);
					while(*p!='\0'&&*p++!='/');
					content_range[2] = std::max<int>(atoi(p),content_range[1]);
				}
				else if(0!=statistics[1])
				{
					assert(0);
					content_range[0] = statistics[0];
					content_range[1] = statistics[0]+statistics[1]-1;
					content_range[2] = content_range[1]+1;
				}				
				if(rngI!=nullptr)				
				rngI->limit_to_size(content_range[2]);

				break;
			}}
			p = strstr(buf,"\r\n\r\n"); 		
			if(p==nullptr) 
			{	
				//Should this retry until there's something
				//conclusive?
				statistics[1] = 0; assert(0);
			}
			else //Clear the header from the buffer?
			{				
				lim = p-buf+4; len = recv(socket,buf,lim,0); 
				assert(lim==len);				
			}
		}
	}
	if(0!=ContentLength())
	{		
		/*TODO: Set up cache.
		FILE *f = thread?_wfopen(incomplete,L"wb"):0;		
		*/

		#ifdef _DEBUG
		daeEH::Warning << "DOWNLOADING " << statistics[1] << " TOTAL...";
		#endif		
	}
	else switch(http_status)
	{
	case 200: case 206: assert(0); //Can GET download 0Bs?

	case 301: //TODO: Redirect.
	case 302:
	case 303:
	case 304: assert(304!=http_status); //Cache follow-up?
	case 305:
	case 307:
	case 308:
	case 412: //Can mean the server modified the file mid-download. NGINX.

	default: resume_disconnect(); 

		daeEH::Error<<"Unhandled HTTP status code: "<<http_status;
	}
}

size_t HTTP_agent::peekIn(void *in, size_t chars) 
{
	if(!connected()) return 0;

	chars = std::min(chars,ContentLength());

	int out = recv(socket,(char*)in,chars,MSG_PEEK);

	if(out==0&&resume()) out = recv(socket,(char*)in,chars,MSG_PEEK);

	return out;
}

daeOK HTTP_agent::readIn(void *in, size_t chars)
{	
	//This is for timing? It could be chars. It's adapted
	//from a buffered loop. But this is really a callback.
	enum{ bufmost = 8192 };
	char *bufp = (char*)in;
		  
	#ifdef _WIN32
	typedef int I;
	#else
	typedef size_t I;
	#endif

	for(size_t remaining=chars;remaining!=0;)
	{
		//hurry up the last bit
		I len = std::min<I>(remaining,bufmost);
		len = recv(socket,bufp,len,0);

		if(0==len) //Resume download?
		{
			//statistics[0] is the total number of bytes so
			//it's not tallied up twice.
			if(statistics[0]<statistics[1])
			{
				if(!resume()) return DAE_ERROR;
			}
		}
		else if(len==~I())
		{
			daeEH::Error<<"WinSock error: "<<len;

			//Don't want to call assert() because the Windows demo
			//is built in debug mode and so will most likely crash.
			#ifdef _WIN32
			#define _(x) case WSA##x:
			switch(len=WSAGetLastError())
			#else
			#define _(x) case x:
			switch(len=errno)
			#endif			
			{
			_(EMSGSIZE) /*assert(0);*/ len = bufmost; break;

			default: /*assert(0);*/ len = 0; break;				
			}
			#undef _
			
		}
		/*TODO: Set up cache.
		fwrite(bufp,len,1,f); 
		*/
		bufp+=len; remaining-=len; statistics[0]+=len; //May be 0.

		if(statistics[0]>=statistics[1])
		{
			assert(statistics[0]==statistics[1]);

			//complete = true; break; //Finish up outside loop.			
			progress(this); return DAE_OK;
		}
		else //update the timing stats?
		{
			RT::Float now = RT::Time();
	
			//THIS WILL BE NEGATIVE/HUGE IF THERE IS WRAPAROUND.
			if(now-pulse>0.25||now<pulse)
			{	
				float seconds = float(now-pulse);
				if(seconds>0)
				statistics[2] = float(statistics[0]-pulse_Bs)/seconds;
				pulse = now; pulse_Bs = statistics[0];				
				if(seconds>0)
				statistics[3]+=int(seconds*1000);
				if(!progress(this))
				{
					//canceled = true; break; 
					return DAE_ERROR;
				}
			}
		}		
	}

	return DAE_OK; //daeIOPlugin may call this in a buffering loop.
}

HTTP_agent::~HTTP_agent()
{
	/*TODO: Set up cache.
	if(f) fclose(f); 
	*/

	if(statistics[0]==statistics[1])
	{
		/*TODO: Set up cache.
		wchar_t rename[MAX_PATH];
		wcscpy(rename,incomplete);
		wchar_t *_incomplete = wcsrchr(rename,'.');
		if(_incomplete&&!wcscmp(_incomplete,L".incomplete")) //paranoia
		{
			*_incomplete = '\0'; //strip .incomplete from file name
		}
		if(!MoveFileW(incomplete,rename)) assert(0);
		*/
	}
	
	resume_disconnect();
}
