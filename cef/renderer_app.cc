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

RendererApp::RendererApp() :
	omnis_handler_(new OmnisHandler())
{
	InitCommandNameMap();
}

bool RendererApp::OmnisHandler::Execute(const CefString& name,
										CefRefPtr<CefV8Value> object,
										const CefV8ValueList& arguments,
										CefRefPtr<CefV8Value>& retval,
										CefString& exception) {
	if(name == "sendOmnis") {
		if (arguments.size() == 2 &&
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
	} else if(name == "setEventCallback") {
		if (arguments.size() == 2 &&
			arguments[0]->IsString() &&
			arguments[1]->IsFunction()) {
			// register the callback function for this event name.
			CefString event_name = arguments[0]->GetStringValue();
			CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
			int browser_id = context->GetBrowser()->GetIdentifier();
			event_callbacks_.insert(
				std::make_pair(
					std::make_pair(event_name, browser_id),
					std::make_pair(context, arguments[1]))
				);
			return true;
		}
	}
	return false;
}

bool RendererApp::OmnisHandler::CustomEvent(CefRefPtr<CefBrowser> browser, const std::wstring& name, const std::wstring& value) {
	int browser_id = browser->GetIdentifier();
	EventCallbackMap::const_iterator it = event_callbacks_.find(
		std::make_pair(name, browser_id));
	if (it != event_callbacks_.end()) {
		// keep a local reference to the objects since the callback may remove itself.
		CefRefPtr<CefV8Context> context = it->second.first;
		CefRefPtr<CefV8Value> callback = it->second.second;

		// enter the context and call the callback.
		context->Enter();
		CefRefPtr<CefV8Value> ret_val;
		/*CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
		CefRefPtr<CefV8Value> parse =
		globalObj->GetValue("JSON")->GetValue("parse");*/
		CefV8ValueList args;
		args.push_back(CefV8Value::CreateString(value));
		ret_val = callback->ExecuteFunction(NULL, args);
		context->Exit();
		return true;
	}
	return false;
}

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
		"  omnis.setEventCallback = function(name, cb) {"
		"    native function setEventCallback();"
		"    if(typeof name !== 'string')"
		"        throw new TypeError('First argument must be string event name.'); "
		"    if(typeof cb !== 'function')"
		"        throw new TypeError('Second argument must be callback function.'); "
		"    return setEventCallback(name, cb);"
		"  };"
		"})();";
	CefRegisterExtension("v8/omnis", code, omnis_handler_);
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
				return true;
			}
			case navigate: {
				// navigate to the given URL.
				CefRefPtr<CefListValue> args = message->GetArgumentList();
				if (args->GetSize() == 1) {
					CefString url = args->GetString(0);
					browser->GetMainFrame()->LoadURL(url);
				}
				else
					throw std::runtime_error("The navigate command needs a single string argument.");
				return true;
			}
			case customEvent: {
				// invoke the callback function for this event, if any.
				CefRefPtr<CefListValue> args = message->GetArgumentList();
				if (args->GetSize() == 1) {
					std::wstring message = args->GetString(0), value;
					// if the message contains a colon, it separates the name from the event value.
					std::wstring::size_type i = message.find(':');
					if (i != std::wstring::npos) {
						message[i] = 0;
						++i;
						if (i < message.size())
							value = &message[i];
						message.resize(i - 1);
					}
					omnis_handler_->CustomEvent(browser, message, value);
				}
				else
					throw std::runtime_error("The navigate command needs a single string argument.");
				return true;
			}
		}
	}
	return false;
}