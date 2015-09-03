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
	ClientHandler(const std::string &pipe_name);
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
		exit
	};
	std::map<std::string, CommandName> command_name_map_;

	void PostPipeMessage(const CefString &name, const CefString &message);

	CefRefPtr<MessagePipeClient> message_pipe_;
	std::string pipe_name_;

	// List of existing browser windows. Only accessed on the CEF UI thread.
	typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
	BrowserList browser_list_;

	bool is_closing_;
	static bool devtools_class_registered_;
	static LRESULT CALLBACK RootWndProc(HWND hWnd, UINT message,
                                        WPARAM wParam, LPARAM lParam);

	void RegisterDevToolsClass();
	void ShowDevTools(CefRefPtr<CefBrowser> browser,
					const CefPoint& inspect_element_at);
	void CloseDevTools(CefRefPtr<CefBrowser> browser);

	// Create a new popup window using the specified information. |is_devtools|
	// will be true if the window will be used for DevTools. Return true to
	// proceed with popup browser creation or false to cancel the popup browser.
	// May be called on any thead.
	bool CreatePopupWindow(
		CefRefPtr<CefBrowser> browser,
		bool is_devtools,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings);

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif  // CEF_TESTS_CEF_CLIENT_HANDLER_H_
