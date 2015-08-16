#ifndef __CLIENT_HANDLER_H__
#define __CLIENT_HANDLER_H__

#include "include/cef_client.h"

class ClientHandler : public CefClient
{
private:
	HWND mHwnd;

public:
	CefRefPtr<CefBrowser> m_Browser;
	int m_BrowserId;

	ClientHandler();
	~ClientHandler();

	int GetBrowserId() 
	{ 
		return m_BrowserId; 
	}
	CefRefPtr<CefBrowser> GetBrowser() 
	{ 
		return m_Browser; 
	}
    
	void OnAfterCreated(CefRefPtr<CefBrowser> browser);

	IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif