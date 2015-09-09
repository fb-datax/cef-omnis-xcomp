// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_handler.h"

#include <sstream>
#include <string>

#include "devtools_handler.h"
#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

typedef rapidjson::GenericStringBuffer<rapidjson::UTF16<>> JSONStringBuffer;
typedef rapidjson::Writer<JSONStringBuffer, rapidjson::UTF16<>> JSONWriter;

namespace {

enum client_menu_ids {
  CLIENT_ID_SHOW_DEVTOOLS   = MENU_ID_USER_FIRST,
  CLIENT_ID_CLOSE_DEVTOOLS,
  CLIENT_ID_INSPECT_ELEMENT
};
ClientHandler* g_instance = NULL;

}  // namespace

ClientHandler::ClientHandler(HWND hwnd, const std::string &pipe_name) : 
	is_closing_(false),
	hwnd_(hwnd),
	pipe_name_(pipe_name) {
	DCHECK(!g_instance);
	g_instance = this;
	InitCommandNameMap();

	if (!pipe_name_.empty()) {
		// connect as a client of the named pipe.
		message_pipe_ = new MessagePipeClient(pipe_name_, this);
		message_pipe_->QueueConnect();
	}
}

ClientHandler::~ClientHandler() {
  g_instance = NULL;
  if(message_pipe_.get())
	message_pipe_->ResetPipeHandler();
}

// static
ClientHandler* ClientHandler::GetInstance() {
  return g_instance;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// add to the list of existing browsers.
	browser_list_.push_back(browser);
	
	// notify the XCOMP that we're ready.
	CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
	std::wstringstream ss;
	ss << std::dec << (int) hwnd;
	PostPipeMessage(L"ready", ss.str());
}

bool ClientHandler::OnProcessMessageReceived( CefRefPtr<CefBrowser> browser,
											  CefProcessId source_process,
											  CefRefPtr<CefProcessMessage> message) {
	CEF_REQUIRE_UI_THREAD();
	CommandNameMap::const_iterator command = command_name_map_.find(message->GetName());
	if(command != command_name_map_.end()) {
		switch(command->second) {
			case sendOmnis:
				CefRefPtr<CefListValue> args = message->GetArgumentList();
				if(args->GetSize() == 2)
					PostPipeMessage(args->GetString(0), args->GetString(1));
				return true;
		}
	}
	return false;
}

void ClientHandler::OnBeforeContextMenu(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model) {
	CEF_REQUIRE_UI_THREAD();

	if((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0) {
		// sdd a separator if the menu already has items.
		if(model->GetCount() > 0)
			model->AddSeparator();

		// add devtools items.
		model->AddItem(CLIENT_ID_SHOW_DEVTOOLS, "&Show DevTools");
		model->AddItem(CLIENT_ID_CLOSE_DEVTOOLS, "Close DevTools");
		model->AddSeparator();
		model->AddItem(CLIENT_ID_INSPECT_ELEMENT, "Inspect Element");
	}
}

bool ClientHandler::OnContextMenuCommand(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    int command_id,
    EventFlags event_flags) {
  CEF_REQUIRE_UI_THREAD();

  switch (command_id) {
    case CLIENT_ID_SHOW_DEVTOOLS:
      ShowDevTools(browser, CefPoint());
      return true;
    case CLIENT_ID_CLOSE_DEVTOOLS:
      CloseDevTools(browser);
      return true;
    case CLIENT_ID_INSPECT_ELEMENT:
      ShowDevTools(browser, CefPoint(params->GetXCoord(), params->GetYCoord()));
      return true;
  }
  return false;
}

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
									CefRefPtr<CefFrame> frame,
									const CefString& url) {
	CEF_REQUIRE_UI_THREAD();
	PostPipeMessage(L"address", url);
}

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
	CEF_REQUIRE_UI_THREAD();
	CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
	SetWindowText(hwnd, std::wstring(title).c_str());
}

void ClientHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser,
									const CefString& value) {
	CEF_REQUIRE_UI_THREAD();
	//PostPipeMessage(L"status", value);
}
bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
									const CefString& message,
									const CefString& source,
									int line) {
	CEF_REQUIRE_UI_THREAD();
    JSONStringBuffer s;
    JSONWriter writer(s);
    writer.StartObject();
	if(!message.empty()) {
		writer.String(L"message");
		writer.String(message.c_str());
	}
	if(!source.empty()) {
		writer.String(L"source");
		writer.String(source.c_str());
	}
	writer.String(L"line");
	writer.Int(line);
    writer.EndObject();
	PostPipeMessage(L"console", s.GetString());
	return false;
}


bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
	CEF_REQUIRE_UI_THREAD();

	// don't display an error for downloaded files.
	if(errorCode == ERR_ABORTED)
		return;
  
	// send JSON message to xcomp.
	JSONStringBuffer s;
	JSONWriter writer(s);
	writer.StartObject();
	if(!errorText.empty()) {
		writer.String(L"errorText");
		writer.String(errorText.c_str());
	}
	if(!failedUrl.empty()) {
		writer.String(L"failedUrl");
		writer.String(failedUrl.c_str());
	}
	writer.String(L"errorCode");
	writer.Int(errorCode);
	writer.EndObject();
	PostPipeMessage(L"loadError", s.GetString());

	// display a load error message.
	std::stringstream ss;
	ss << "<html><body bgcolor=\"white\">"
		"<h2>Failed to load URL " << std::string(failedUrl) <<
		" with error " << std::string(errorText) << " (" << errorCode <<
		").</h2></body></html>";
	frame->LoadString(ss.str(), failedUrl);
}

void ClientHandler::InitCommandNameMap() {
	command_name_map_["execute"] = execute;
	command_name_map_["navigate"] = navigate;
	command_name_map_["sendOmnis"] = sendOmnis;
	command_name_map_["customEvent"] = customEvent;
	command_name_map_["resize"] = resize;
	command_name_map_["focus"] = focus;
	command_name_map_["exit"] = exit;
}

void ClientHandler::OnReadCompleted(std::wstring &buffer) {
	//MessageBox(NULL, buffer.c_str(), L"OnReadCompleted", MB_OK);
	CefRefPtr<CefProcessMessage> message = ToProcessMessage(buffer);
	CommandNameMap::const_iterator command = command_name_map_.find(message->GetName());
	if(command != command_name_map_.end()) {
		switch(command->second) {
			case execute:
			case navigate:
			case customEvent: {
				// pass the message to the renderer process of each browser.
				BrowserList::iterator bit = browser_list_.begin();
				for (; bit != browser_list_.end(); ++bit)
					(*bit)->SendProcessMessage(PID_RENDERER, message);
				break;
			}
			case resize: {
				RECT rect;
				if(GetClientRect(hwnd_, &rect)) {
					BrowserList::iterator bit = browser_list_.begin();
					for (; bit != browser_list_.end(); ++bit) {
						HWND h = (*bit)->GetHost()->GetWindowHandle();
						SetWindowPos(h, NULL, 0, 0, rect.right, rect.bottom, 
							SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER);
					}
				}
				break;
			}
			case focus: {
				BrowserList::iterator bit = browser_list_.begin();
				for (; bit != browser_list_.end(); ++bit)
					(*bit)->GetHost()->SetFocus(true);
				break;
			}
			case exit: {
				CloseAllBrowsers(true);
				break;
			}
		}
	}
}

void ClientHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
        base::Bind(&ClientHandler::CloseAllBrowsers, this, force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
    (*it)->GetHost()->CloseBrowser(force_close);
}

void ClientHandler::PostPipeMessage(const CefString &name, const CefString &message) {
	CEF_REQUIRE_UI_THREAD();
	/*if(!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI, NewCefRunnableMethod(this, &BrowserApp::PostPipeMessage, name, message));
		return;
	}*/
	if(message_pipe_.get())
		message_pipe_->QueueWrite(name, message);
}

void ClientHandler::ShowDevTools(CefRefPtr<CefBrowser> browser,
                                 const CefPoint& inspect_element_at) {
	CefWindowInfo window_info;
	//window_info.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
	window_info.SetAsPopup(NULL, "DevTools");
	CefRefPtr<CefClient> client = new DevToolsHandler();
	CefBrowserSettings settings;
	browser->GetHost()->ShowDevTools(window_info, client, settings, inspect_element_at);
}

void ClientHandler::CloseDevTools(CefRefPtr<CefBrowser> browser) {
  browser->GetHost()->CloseDevTools();
}