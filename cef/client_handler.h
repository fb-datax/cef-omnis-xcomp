#ifndef CEF_TESTS_CEF_CLIENT_HANDLER_H_
#define CEF_TESTS_CEF_CLIENT_HANDLER_H_

#include "include/cef_client.h"
#include "message_pipe.h"

#include <list>

class ClientHandler : public CefClient,
                      public CefContextMenuHandler,
                      public CefDisplayHandler,
					  public CefFocusHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
					  public MessagePipe::PipeOperationHandler {
 public:
	ClientHandler(HWND hwnd, const std::string &pipe_name);
	~ClientHandler();

	// provide access to the single global instance of this object.
	static ClientHandler* GetInstance();

	// CefClient methods:
	CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefFocusHandler> GetFocusHandler() OVERRIDE {
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

	// CefFocusHandler methods:
	/*virtual void OnTakeFocus(CefRefPtr<CefBrowser> browser,
							bool next) OVERRIDE {
		PostPipeMessage(L"CEF onTakeFocus", L"");
	}
	virtual bool OnSetFocus(CefRefPtr<CefBrowser> browser,
							FocusSource source) OVERRIDE {
		PostPipeMessage(L"CEF onSetFocus", L"");
		return false;
	}*/
	virtual void OnGotFocus(CefRefPtr<CefBrowser> browser) OVERRIDE {
		//PostPipeMessage(L"CEF onGotFocus", L"");
		PostPipeMessage(L"gotFocus", L"");
	}

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
		message_pipe_->QueueRead();
	}
	virtual void OnConnectFailed(DWORD err) OVERRIDE {
		throw std::runtime_error("Connect main pipe");
	}
	virtual void OnReadCompleted(std::wstring &buffer) OVERRIDE;
	virtual void OnWriteCompleted(std::wstring &buffer) OVERRIDE { }

	// request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);

	bool IsClosing() const { return is_closing_; }

 private:

	enum CommandName {
		execute,
		navigate,
		sendOmnis,
		customEvent,
		contextMenus,
		resize,
		focus,
		exit
	};
	typedef std::map<std::string, CommandName> CommandNameMap;
	CommandNameMap command_name_map_;
	void InitCommandNameMap();

	void PostPipeMessage(const CefString &name, const CefString &message);

	HWND hwnd_;
	CefRefPtr<MessagePipeClient> message_pipe_;
	std::string pipe_name_;

	// list of existing browser windows. Only accessed on the CEF UI thread.
	typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
	BrowserList browser_list_;

	bool is_closing_;
	bool context_menus_;

	void RegisterDevToolsClass();
	void ShowDevTools(CefRefPtr<CefBrowser> browser,
					  const CefPoint& inspect_element_at);
	void CloseDevTools(CefRefPtr<CefBrowser> browser);

	// include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif  // CEF_TESTS_CEF_CLIENT_HANDLER_H_
