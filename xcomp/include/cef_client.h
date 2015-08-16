#ifndef __cef_client__h__
#define __cef_client__h__

#include "cef.h"
#include "client_handler.h"




class cef_client
{
private:
	HWND			mHWnd;
	HINSTANCE       mInstance;
	qshort		mIconColumn;
	HWND		mClientHwnd;
	//CefRefPtr<ClientHandler> g_handler;

	CefWindowInfo info;
    CefBrowserSettings settings;
	CefSettings cefsettings;
	//CefRefPtr<CefClient> client;
	
public:
	cef_client( HWND pFieldHWnd, HINSTANCE pInstance );
	~cef_client();
	void cef_create();
	void cef_paint();
	//qlong attributeSupport( LPARAM pMessage, WPARAM wParam, LPARAM lParam, EXTCompInfo* eci );
	//qbool drawSingleLine(EXTListLineInfo* pLineInfo,EXTCompInfo* pEci);  
};

#endif
