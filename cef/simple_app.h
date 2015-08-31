// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEF_SIMPLE_APP_H_
#define CEF_TESTS_CEF_SIMPLE_APP_H_

#include "message_pipe.h"
#include "include/cef_app.h"

class SimpleApp : public CefApp,
                  public CefBrowserProcessHandler,
                  public CefRenderProcessHandler {
  public:
	SimpleApp();

	// CefApp methods:
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
		OVERRIDE { return this; }

	// CefBrowserProcessHandler methods:
	virtual void OnContextInitialized() OVERRIDE;

	// CefRenderProcessHandler methods:
	virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

	
	virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE {
		return;
	}

 private:
	 
	class MainMessagePipeClient : public MessagePipeClient, public MessagePipe::PipeOperationHandler {
	public:
		MainMessagePipeClient(CefString main_pipe_name) : 
		  MessagePipeClient(main_pipe_name, this) {
		}
		virtual void OnConnectCompleted() OVERRIDE {
			MessageBox(NULL, L"Connect", L"Stop", MB_OK);
			QueueRead();
		}
		virtual void OnConnectFailed(DWORD err) OVERRIDE {
			throw std::runtime_error("Connect main pipe");
		}
		virtual void OnReadCompleted(std::wstring &buffer) OVERRIDE {
			MessageBox(NULL, buffer.c_str(), L"OnReadCompleted", MB_OK);
		}
		virtual void OnWriteCompleted(std::wstring &buffer) OVERRIDE { }
	};

	void PostMessage(const CefString &name, const CefString &message);

	CefRefPtr<MainMessagePipeClient> message_pipe_;
	HWND hwnd_;
	std::string pipe_name_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif  // CEF_TESTS_CEF_SIMPLE_APP_H_
