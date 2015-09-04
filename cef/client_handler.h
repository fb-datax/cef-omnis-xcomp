#ifndef CEF_TESTS_CEF_CLIENT_HANDLER_H_
#define CEF_TESTS_CEF_CLIENT_HANDLER_H_

#include "include/cef_client.h"
#include "message_pipe.h"

#include <list>

class ClientHandler : public CefClient,
                      public CefContextMenuHandler,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
					  public MessagePipe::PipeOperationHandler {
 public:
	ClientHandler(HWND hwnd, const std::string &pipe_name);
	~ClientHandler();

	// Provide access to the single global instance of this object.
	static ClientHandler* GetInstance();

	// CefClient methods:
	CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
		return this;
	}
	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
										CefProcessId source_process,
										CefRefPtr<CefProcessMessage> message) OVERRIDE;

	// CefContextMenuHandler methods
	void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
							CefRefPtr<CefFrame> frame,
							CefRefPtr<CefContextMenuParams> params,
							CefRefPtr<CefMenuModel> model) OVERRIDE;
	bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
							CefRefPtr<CefFrame> frame,
							CefRefPtr<CefContextMenuParams> params,
							int command_id,
							EventFlags event_flags) OVERRIDE;

	// CefDisplayHandler methods:
	virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								const CefString& url) OVERRIDE;
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
								const CefString& title) OVERRIDE;
	virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser,
								const CefString& value) OVERRIDE;
	virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
								const CefString& message,
								const CefString& source,
								int line) OVERRIDE;

	// CefLifeSpanHandler methods:
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

	// CefLoadHandler methods:
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
							CefRefPtr<CefFrame> frame,
							ErrorCode errorCode,
							const CefString& errorText,
							const CefString& failedUrl) OVERRIDE;
  

	 // PipeOperationHandler methods:
	virtual void OnConnectCompleted() OVERRIDE {
		//MessageBox(NULL, L"Connect", L"Stop", MB_OK);
		message_pipe_->QueueRead();
	}
	virtual void OnConnectFailed(DWORD err) OVERRIDE {
		throw std::runtime_error("Connect main pipe");
	}
	virtual void OnReadCompleted(std::wstring &buffer) OVERRIDE;
	virtual void OnWriteCompleted(std::wstring &buffer) OVERRIDE { }

	// Request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);

	bool IsClosing() const { return is_closing_; }

 private:

	enum CommandName {
		execute,
		navigate,
		sendOmnis,
		resize,
		exit
	};
	typedef std::map<std::string, CommandName> CommandNameMap;
	CommandNameMap command_name_map_;
	void InitCommandNameMap();

	void PostPipeMessage(const CefString &name, const CefString &message);

	HWND hwnd_;
	CefRefPtr<MessagePipeClient> message_pipe_;
	std::string pipe_name_;

	// List of existing browser windows. Only accessed on the CEF UI thread.
	typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
	BrowserList browser_list_;

	bool is_closing_;

	void RegisterDevToolsClass();
	void ShowDevTools(CefRefPtr<CefBrowser> browser,
					const CefPoint& inspect_element_at);
	void CloseDevTools(CefRefPtr<CefBrowser> browser);

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif  // CEF_TESTS_CEF_CLIENT_HANDLER_H_
