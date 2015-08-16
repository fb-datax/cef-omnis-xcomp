#include "cef_client.h"


#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "HWND.HE"

CefRefPtr<ClientHandler> g_handler;

cef_client::cef_client( HWND pFieldHWnd, HINSTANCE pInstance )
{
	mHWnd = pFieldHWnd;
	mInstance = pInstance;
	mIconColumn = 1;
    

}

void cef_client::cef_create()
{
	CefMainArgs main_args(mInstance);
    //int exit = CefExecuteProcess(main_args, NULL);

	cefsettings.multi_threaded_message_loop = true;
    //CefInitialize(main_args, cefsettings, NULL);
	CefRefPtr<CefApp> cefApplication;

    CefRefPtr<CefClient> client(new ClientHandler());
    g_handler = (ClientHandler*) client.get();

	CefInitialize(main_args, cefsettings, cefApplication, NULL);

	//HWND parentHwnd = WNDgetParent( mHWnd );
	qrect numberRect;
	WNDgetClientRect( mHWnd, &numberRect );
    
	qulong style = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE;
    WNDborderStruct border(WND_BORD_EMBOSSED);
    mClientHwnd = WNDcreateWindow( mHWnd, style, 0, WNDgetProcInst(mHWnd), &numberRect, &border);

	//mClientHwnd = WNDaddWindowComponent( mHWnd, WND_WC_CLIENT, style, WND_DRAGBORDER, WNDgetProcInst(mHWnd), 0, &border ); 
    RECT rect;
	GetClientRect(mClientHwnd, &rect);

    // Initialize window info to the defaults for a child window
	
    info.SetAsChild(mClientHwnd, rect);
    

    // Create the new child browser window
	bool opened = false;
	opened = CefBrowserHost::CreateBrowser(info, client.get(),"http://google.com", settings, NULL);
    //CefDoMessageLoopWork();
}

void cef_client::cef_paint()
{
	//CefDoMessageLoopWork();
}

cef_client::~cef_client()
{
	CefShutdown();
}



