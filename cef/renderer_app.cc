// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "renderer_app.h"

#include <string>

#include "client_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_runnable.h"
#include "include/wrapper/cef_helpers.h"
#include "include/base/cef_bind.h"

RendererApp::RendererApp() {
	command_name_map_["execute"] = execute;
}

void RendererApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser) {
	//MessageBox(NULL, L"OnBrowserCreated", L"Stop", MB_OK);
	/*CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("ready");
	browser->SendProcessMessage(PID_BROWSER, message);*/
	return;
}

bool RendererApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
										   CefProcessId source_process,
										   CefRefPtr<CefProcessMessage> message) {
	std::wstring msg = L"OnProcessMessageReceived (renderer): ";
	msg += message->GetName();
	//MessageBox(NULL, msg.c_str(), L"Stop", MB_OK);
	switch(command_name_map_[message->GetName()]) {
		case execute:
			// execute the given javascript code.
			CefRefPtr<CefListValue> args = message->GetArgumentList();
			if(args->GetSize() == 1) {
				CefString expression = args->GetString(0);
				CefRefPtr<CefFrame> frame = browser->GetMainFrame();
				//frame->ExecuteJavaScript("alert('ExecuteJavaScript works!');", frame->GetURL(), 0);
				frame->ExecuteJavaScript(expression, frame->GetURL(), 0);
				//frame->LoadURL("http://google.com");
			} else
				throw std::runtime_error("The execute command needs a single string argument.");
			//MessageBox(NULL, L"execute", L"Stop", MB_OK);
	}
	return false;
}