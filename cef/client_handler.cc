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

namespace {

enum client_menu_ids {
  CLIENT_ID_SHOW_DEVTOOLS   = MENU_ID_USER_FIRST,
  CLIENT_ID_CLOSE_DEVTOOLS,
  CLIENT_ID_INSPECT_ELEMENT
};
ClientHandler* g_instance = NULL;

}  // namespace

// static
LRESULT CALLBACK ClientHandler::RootWndProc(HWND hWnd, UINT message,
                                            WPARAM wParam, LPARAM lParam) {
  /*REQUIRE_MAIN_THREAD();

  // Callback for the main window
  switch (message) {
    case WM_COMMAND:
      if (self->OnCommand(LOWORD(wParam)))
        return 0;
      break;

    case WM_PAINT:
      self->OnPaint();
      return 0;

    case WM_SETFOCUS:
      self->OnFocus();
      return 0;

    case WM_SIZE:
      self->OnSize(wParam == SIZE_MINIMIZED);
      break;

    case WM_MOVING:
    case WM_MOVE:
      self->OnMove();
      return 0;

    case WM_ERASEBKGND:
      if (self->OnEraseBkgnd())
        break;
      // Don't erase the background.
      return 0;

    case WM_ENTERMENULOOP:
      if (!wParam) {
        // Entering the menu loop for the application menu.
        CefSetOSModalLoop(true);
      }
      break;

    case WM_EXITMENULOOP:
      if (!wParam) {
        // Exiting the menu loop for the application menu.
        CefSetOSModalLoop(false);
      }
      break;

    case WM_CLOSE:
      if (self->OnClose())
        return 0;  // Cancel the close.
      break;

    case WM_NCHITTEST: {
      LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
      if (hit == HTCLIENT) {
        POINTS points = MAKEPOINTS(lParam);
        POINT point = { points.x, points.y };
        ::ScreenToClient(hWnd, &point);
        if (::PtInRegion(self->draggable_region_, point.x, point.y)) {
          // If cursor is inside a draggable region return HTCAPTION to allow
          // dragging.
          return HTCAPTION;
        }
      }
      return hit;
    }

    case WM_NCDESTROY:
      // Clear the reference to |self|.
      SetUserDataPtr(hWnd, NULL);
      self->hwnd_ = NULL;
      self->OnDestroyed();
      return 0;
  }*/

  return DefWindowProc(hWnd, message, wParam, lParam);
}

bool ClientHandler::devtools_class_registered_ = false;

ClientHandler::ClientHandler(const std::string &pipe_name) : 
	is_closing_(false),
	pipe_name_(pipe_name) {
	DCHECK(!g_instance);
	g_instance = this;
	
	command_name_map_["execute"] = execute;
	command_name_map_["exit"] = exit;

	if (!pipe_name_.empty()) {
		// Connect as a client of the named pipe.
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

	// Add to the list of existing browsers.
	browser_list_.push_back(browser);
	
	//MessageBox(NULL, L"OnAfterCreated", L"Stop", MB_OK);
	PostPipeMessage(L"ready", L"");
	PostPipeMessage(L"OnAfterCreated", L"");
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
	PostPipeMessage(L"status", value);
}
bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
									const CefString& message,
									const CefString& source,
									int line) {
	CEF_REQUIRE_UI_THREAD();
	std::wstringstream ss;
	ss << L"[" << source.c_str() << L":" << line << L"] " << message.c_str();
	PostPipeMessage(L"console", ss.str());
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
		case execute: {
			// pass the message to the renderer process of each browser.
			BrowserList::iterator bit = browser_list_.begin();
			for (; bit != browser_list_.end(); ++bit)
				(*bit)->SendProcessMessage(PID_RENDERER, message);
			break;
		}
		case exit: {
			CloseAllBrowsers(true);
			break;
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

void ClientHandler::RegisterDevToolsClass() {
	/*if(!devtools_class_registered_) {
		devtools_class_registered_ = true;		
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style         = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc   = RootWndProc;
		wcex.cbClsExtra    = 0;
		wcex.cbWndExtra    = 0;
		wcex.hInstance     = hInstance;
		wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CEFCLIENT));
		wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = background_brush;
		wcex.lpszMenuName  = MAKEINTRESOURCE(IDC_CEFCLIENT);
		wcex.lpszClassName = window_class.c_str();
		wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		RegisterClassEx(&wcex);
	}*/
}

void ClientHandler::ShowDevTools(CefRefPtr<CefBrowser> browser,
                                 const CefPoint& inspect_element_at) {
	CefWindowInfo window_info;
	window_info.SetAsPopup(browser->GetHost()->GetWindowHandle(), "devtools");
	CefRefPtr<CefClient> client = new DevToolsHandler();
	CefBrowserSettings settings;
	CefRefPtr<DevToolsHandler> handler = new DevToolsHandler();
	//const std::wstring& window_class = L"IDC_DEVTOOLS";
	//HINSTANCE hInstance = GetModuleHandle(NULL);
	//RegisterRootClass(hInstance, window_class, background_brush);

	//if(CefBrowserHost::CreateBrowser(window_info, handler, std::string(), settings, NULL))
		browser->GetHost()->ShowDevTools(window_info, client, settings, inspect_element_at);
}

void ClientHandler::CloseDevTools(CefRefPtr<CefBrowser> browser) {
  browser->GetHost()->CloseDevTools();
}

bool ClientHandler::CreatePopupWindow(
		CefRefPtr<CefBrowser> browser,
		bool is_devtools,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings) {
	// Note: This method will be called on multiple threads.

	// The popup browser will be parented to a new native window.
	// Don't show URL bar and navigation buttons on DevTools windows.
	//MainContext::Get()->GetRootWindowManager()->CreateRootWindowAsPopup(
	//	!is_devtools, is_osr(), popupFeatures, windowInfo, client, settings);

	return true;
}