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
	virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE { }
	virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE {
		//MessageBox(NULL, L"OnBrowserDestroyed", L"Stop", MB_OK);
	}	
	virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE {
		//MessageBox(NULL, L"OnRenderThreadCreated", L"Stop", MB_OK);
	}
	virtual void OnWebKitInitialized() OVERRIDE;
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
								CefRefPtr<CefV8Context> context) OVERRIDE;
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

	 class OmnisHandler : public CefV8Handler {
	 public:
		 OmnisHandler();
		 virtual bool Execute(const CefString& name,
			 CefRefPtr<CefV8Value> object,
			 const CefV8ValueList& arguments,
			 CefRefPtr<CefV8Value>& retval,
			 CefString& exception) OVERRIDE;
		 bool CustomEvent(CefRefPtr<CefBrowser> browser, const std::wstring& name, const std::wstring& value);
		 void ReleaseContext(CefRefPtr<CefV8Context> context);
	 protected:
		 // custom message callbacks.
		 typedef std::map<std::pair<std::wstring, int>,
			 std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value> > >
			 EventCallbackMap;
		 EventCallbackMap event_callbacks_;

		 enum CommandName {
			 sendOmnis,
			 setEventCallback,
			 clearEventCallback
		 };
		 typedef std::map<std::string, CommandName> CommandNameMap;
		 CommandNameMap command_name_map_;
		 void InitCommandNameMap();

		 // Provide the reference counting implementation for this class.
		 IMPLEMENT_REFCOUNTING(OmnisHandler);
	 };

	 CefRefPtr<OmnisHandler> omnis_handler_;

	// the command name map allows for an efficient string switch statement.
	enum CommandName {
		execute,
		navigate,
		customEvent,
	};
	typedef std::map<std::string, CommandName> CommandNameMap;
	CommandNameMap command_name_map_;
	void InitCommandNameMap() {
		command_name_map_["execute"] = execute;
		command_name_map_["navigate"] = navigate;
		command_name_map_["customEvent"] = customEvent;
	}

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(RendererApp);
};

#endif  // CEF_TESTS_CEF_RENDERER_APP_H_
