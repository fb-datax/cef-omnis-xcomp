#ifndef CEF_TESTS_CEF_BROWSER_APP_H_
#define CEF_TESTS_CEF_BROWSER_APP_H_

#include "message_pipe.h"
#include "include/cef_app.h"

#define BREAKPOINT __asm int 3;

class BrowserApp : public CefApp,
                  public CefBrowserProcessHandler {
  public:
	BrowserApp();

	// CefApp methods:
	CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE {
		return this;
	}

	// CefBrowserProcessHandler methods:
	virtual void OnContextInitialized() OVERRIDE;

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

	void PostPipeMessage(const CefString &name, const CefString &message);

	CefRefPtr<MainMessagePipeClient> message_pipe_;
	HWND hwnd_;
	std::string pipe_name_;

	enum CommandName {
		ready
	};
	std::map<std::string, CommandName> command_name_map_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(BrowserApp);
};

#endif  // CEF_TESTS_CEF_BROWSER_APP_H_
