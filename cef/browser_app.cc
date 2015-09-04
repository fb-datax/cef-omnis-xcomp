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

	CefWindowInfo window_info;
	window_info.SetAsPopup(NULL, "cef");

	CefBrowserSettings browser_settings;

	/*browser_settings.web_security = STATE_DISABLED;
	browser_settings.universal_access_from_file_urls = STATE_ENABLED;
	browser_settings.file_access_from_file_urls = STATE_ENABLED;
	browser_settings.local_storage = STATE_ENABLED;
	browser_settings.databases = STATE_ENABLED;
	browser_settings.application_cache = STATE_DISABLED;*/


	// check if a "--url=" value was provided via the command-line. otherwise
	// use the default url.
	std::string url;
	CefRefPtr<CefCommandLine> command_line =
		CefCommandLine::GetGlobalCommandLine();
	url = command_line->GetSwitchValue("url");
	if(url.empty())
		url = "http://www.google.com";

	// check if a "--parent-hwnd=" value was provided.
	std::string hwnd_s = command_line->GetSwitchValue("parent-hwnd");
	if(!hwnd_s.empty()) {
		hwnd_ = (HWND) atoi(hwnd_s.c_str());
		RECT rect;
		if(GetClientRect(hwnd_, &rect))
			window_info.SetAsChild(hwnd_, rect);
	}

	// Check if a "--pipe-name=" value was provided.
	std::string pipe_name = command_line->GetSwitchValue("pipe-name");

	// ClientHandler implements browser-level callbacks.
	CefRefPtr<ClientHandler> handler(new ClientHandler(hwnd_, pipe_name));

	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, handler.get(), url,
								  browser_settings, NULL);
}