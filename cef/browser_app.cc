// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "browser_app.h"

#include <string>

#include "client_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_runnable.h"
#include "include/wrapper/cef_helpers.h"
#include "include/base/cef_bind.h"

BrowserApp::BrowserApp() :
	hwnd_(NULL) {
}

void BrowserApp::OnContextInitialized() {
	CEF_REQUIRE_UI_THREAD();

	// Information used when creating the native window.
	CefWindowInfo window_info;

	//MessageBox(NULL, L"Stop", L"Stop", MB_OK);

	#if defined(OS_WIN)
	// On Windows we need to specify certain flags that will be passed to
	// CreateWindowEx().
	window_info.SetAsPopup(NULL, "cef");
	#endif

	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	std::string url;

	// Check if a "--url=" value was provided via the command-line. If so, use
	// that instead of the default URL.
	CefRefPtr<CefCommandLine> command_line =
		CefCommandLine::GetGlobalCommandLine();
	url = command_line->GetSwitchValue("url");
	if (url.empty())
	url = "http://www.google.com";

	// Check if a "--parent-hwnd=" value was provided.
	std::string hwnd_s = command_line->GetSwitchValue("parent-hwnd");
	if (!hwnd_s.empty()) {
		hwnd_ = (HWND) atoi(hwnd_s.c_str());
		RECT rect;
		if (GetWindowRect(hwnd_, &rect)) {
			rect.right -= rect.left;
			rect.bottom -= rect.top;
			rect.left = rect.top = 0;
			window_info.SetAsChild(hwnd_, rect);
		}
	}

	// Check if a "--pipe-name=" value was provided.
	std::string pipe_name = command_line->GetSwitchValue("pipe-name");

	// ClientHandler implements browser-level callbacks.
	CefRefPtr<ClientHandler> handler(new ClientHandler(pipe_name));

	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, handler.get(), url,
								  browser_settings, NULL);
}