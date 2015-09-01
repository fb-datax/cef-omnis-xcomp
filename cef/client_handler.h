#ifndef CEF_TESTS_CEF_CLIENT_HANDLER_H_
#define CEF_TESTS_CEF_CLIENT_HANDLER_H_

#include "include/cef_client.h"
#include "message_pipe.h"

#include <list>

class ClientHandler : public CefClient,
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

	// CefDisplayHandler methods:
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
								const CefString& title) OVERRIDE;

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
		execute
	};
	std::map<std::string, CommandName> command_name_map_;

	void PostPipeMessage(const CefString &name, const CefString &message);

	CefRefPtr<MessagePipeClient> message_pipe_;
	std::string pipe_name_;

	// List of existing browser windows. Only accessed on the CEF UI thread.
	typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
	BrowserList browser_list_;

	bool is_closing_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif  // CEF_TESTS_CEF_CLIENT_HANDLER_H_
