#ifndef CEF_TESTS_CEF_BROWSER_APP_H_
#define CEF_TESTS_CEF_BROWSER_APP_H_

#include "include/cef_app.h"

#define BREAKPOINT __asm int 3;

class BrowserApp : public CefApp,
                  public CefBrowserProcessHandler {
  public:
	BrowserApp();

	// CefApp methods:
	CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE {
		return this;
	}

	// CefBrowserProcessHandler methods:
	virtual void OnContextInitialized() OVERRIDE;

 private:
	HWND hwnd_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(BrowserApp);
};

#endif  // CEF_TESTS_CEF_BROWSER_APP_H_
