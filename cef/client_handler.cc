// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_handler.h"

#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

namespace {

ClientHandler* g_instance = NULL;

}  // namespace

ClientHandler::ClientHandler(const std::string &pipe_name) : 
	is_closing_(false),
	pipe_name_(pipe_name) {
	DCHECK(!g_instance);
	g_instance = this;

	command_name_map_["execute"] = execute;

	if (!pipe_name.empty()) {
		// Connect as a client of the named pipe.
		message_pipe_ = new MessagePipeClient(pipe_name_, this);
		message_pipe_->QueueConnect();
	}
}

ClientHandler::~ClientHandler() {
  g_instance = NULL;
  message_pipe_->ResetPipeHandler();
}

// static
ClientHandler* ClientHandler::GetInstance() {
  return g_instance;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Add to the list of existing browsers.
	browser_list_.push_back(browser);
	
	//MessageBox(NULL, L"OnAfterCreated", L"Stop", MB_OK);
	PostPipeMessage(L"ready", L"");
	/*CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("ready");
	browser->SendProcessMessage(PID_RENDERER, message);*/
}

bool ClientHandler::OnProcessMessageReceived( CefRefPtr<CefBrowser> browser,
											  CefProcessId source_process,
											  CefRefPtr<CefProcessMessage> message) {
	CEF_REQUIRE_UI_THREAD();
	std::wstring msg = L"OnProcessMessageReceived (browser): ";
	msg += message->GetName();
	MessageBox(NULL, msg.c_str(), L"Stop", MB_OK);
	return false;
}

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
  SetWindowText(hwnd, std::wstring(title).c_str());
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

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL " << std::string(failedUrl) <<
        " with error " << std::string(errorText) << " (" << errorCode <<
        ").</h2></body></html>";
  frame->LoadString(ss.str(), failedUrl);
}

void ClientHandler::OnReadCompleted(std::wstring &buffer) {
	//MessageBox(NULL, buffer.c_str(), L"OnReadCompleted", MB_OK);
	CefRefPtr<CefProcessMessage> message = ToProcessMessage(buffer);
	switch(command_name_map_[message->GetName()]) {
		case execute:
			// pass the message to the renderer process of each browser.
			BrowserList::iterator bit = browser_list_.begin();
			for (; bit != browser_list_.end(); ++bit)
				(*bit)->SendProcessMessage(PID_RENDERER, message);
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