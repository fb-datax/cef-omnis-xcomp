// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEF_RENDERER_APP_H_
#define CEF_TESTS_CEF_RENDERER_APP_H_

#include "include/cef_app.h"

class RendererApp : public CefApp,
					public CefRenderProcessHandler {
  public:
	RendererApp();

	// CefApp methods:
	CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE {
		return this;
	}

	// CefRenderProcessHandler methods:
	virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE {
		//MessageBox(NULL, L"OnBrowserDestroyed", L"Stop", MB_OK);
	}	
	virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE {
		//MessageBox(NULL, L"OnRenderThreadCreated", L"Stop", MB_OK);
	}
	virtual void OnWebKitInitialized() OVERRIDE {
		//MessageBox(NULL, L"OnWebKitInitialized", L"Stop", MB_OK);
	}
	virtual bool OnBeforeNavigation(CefRefPtr<CefBrowser> browser,
									CefRefPtr<CefFrame> frame,
									CefRefPtr<CefRequest> request,
									NavigationType navigation_type,
									bool is_redirect) OVERRIDE { 
		//MessageBox(NULL, L"OnBeforeNavigation", L"Stop", MB_OK);
		return false;
	}
	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								CefRefPtr<CefV8Context> context) OVERRIDE { 
		//MessageBox(NULL, L"OnContextCreated", L"Stop", MB_OK);
	}
	virtual void OnContextReleased(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								CefRefPtr<CefV8Context> context) OVERRIDE { 
		//MessageBox(NULL, L"OnContextReleased", L"Stop", MB_OK);
	}
	virtual void OnUncaughtException(CefRefPtr<CefBrowser> browser,
									CefRefPtr<CefFrame> frame,
									CefRefPtr<CefV8Context> context,
									CefRefPtr<CefV8Exception> exception,
									CefRefPtr<CefV8StackTrace> stackTrace) OVERRIDE { 
		MessageBox(NULL, L"OnUncaughtException", L"Stop", MB_OK);
	}
	virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
									CefRefPtr<CefFrame> frame,
									CefRefPtr<CefDOMNode> node) OVERRIDE { 
		//MessageBox(NULL, L"OnFocusedNodeChanged", L"Stop", MB_OK);
	}
	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
										CefProcessId source_process,
										CefRefPtr<CefProcessMessage> message) OVERRIDE;

 private:

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(RendererApp);
};

#endif  // CEF_TESTS_CEF_RENDERER_APP_H_
