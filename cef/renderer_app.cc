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
	InitCommandNameMap();
}

class SendOmnisHandler : public CefV8Handler {
public:
	virtual bool Execute(const CefString& name,
						CefRefPtr<CefV8Value> object,
						const CefV8ValueList& arguments,
						CefRefPtr<CefV8Value>& retval,
						CefString& exception) OVERRIDE {
	if(name == "sendOmnis" && arguments.size() == 2 && 
			arguments[0]->IsString() && 
			arguments[1]->IsString()) {
		// send the message to the Omnis xcomp.
		CefRefPtr<CefBrowser> browser = CefV8Context::GetCurrentContext()->GetBrowser();
		CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("sendOmnis");
		message->GetArgumentList()->SetString(0, arguments[0]->GetStringValue());
		message->GetArgumentList()->SetString(1, arguments[1]->GetStringValue());
		browser->SendProcessMessage(PID_BROWSER, message);
		return true;
	}
	return false;
	}

	// Provide the reference counting implementation for this class.
	IMPLEMENT_REFCOUNTING(SendOmnisHandler);
};

void RendererApp::OnWebKitInitialized() {
	// define the omnis extension.
	std::string code =
		"var omnis;"
		"if(!omnis)"
		"  omnis = {};"
		"(function() {"
		"  omnis.showMsg = function() {"
		"    native function sendOmnis();"
		"    var args = Array.prototype.slice.call(arguments);"
		"    return sendOmnis('showMsg', JSON.stringify(args));"
		"  };"
		"  omnis.closeModule = function() {"
		"    native function sendOmnis();"
		"    return sendOmnis('closeModule', '');"
		"  };"
		"  omnis.customEvent = function(name) {"
		"    native function sendOmnis();"
		"    var args = Array.prototype.slice.call(arguments, 1);"
		"    args = [String(name)].concat(args.map(String));"
		"    return sendOmnis('customEvent', JSON.stringify(args));"
		"  };"
		"})();";
	CefRegisterExtension("v8/omnis", code, new SendOmnisHandler());
}

bool RendererApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
										   CefProcessId source_process,
										   CefRefPtr<CefProcessMessage> message) {
	CommandNameMap::const_iterator command = command_name_map_.find(message->GetName());
	if(command != command_name_map_.end()) {
		switch(command->second) {
			case execute: {
				// execute the given javascript code.
				CefRefPtr<CefListValue> args = message->GetArgumentList();
				if(args->GetSize() == 1) {
					CefString expression = args->GetString(0);
					CefRefPtr<CefFrame> frame = browser->GetMainFrame();
					frame->ExecuteJavaScript(expression, frame->GetURL(), 0);
				} else
					throw std::runtime_error("The execute command needs a single string argument.");
				break;
			}
			case navigate: {
				// navigate to the given URL.
				CefRefPtr<CefListValue> args = message->GetArgumentList();
				if(args->GetSize() == 1) {
					CefString url = args->GetString(0);
					browser->GetMainFrame()->LoadURL(url);
				} else
					throw std::runtime_error("The navigate command needs a single string argument.");
				break;
			}
		}
	}
	return false;
}