// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "renderer_app.h"

#include <string>

#include "simple_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_runnable.h"
#include "include/wrapper/cef_helpers.h"
#include "include/base/cef_bind.h"

RendererApp::RendererApp() {
}

void RendererApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser) {
	MessageBox(NULL, L"OnBrowserCreated", L"Stop", MB_OK);
	CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("ready");
	browser->SendProcessMessage(PID_BROWSER, message);
	return;
}

bool RendererApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
									CefProcessId source_process,
									CefRefPtr<CefProcessMessage> message) {
	std::wstring msg = L"OnProcessMessageReceived (renderer): ";
	msg += message->GetName();
	MessageBox(NULL, msg.c_str(), L"Stop", MB_OK);
	return false;
}