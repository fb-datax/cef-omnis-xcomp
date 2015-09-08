//
// CefInstance spawns a CefWebLib.exe process and creates a named pipe server to
// communicate with it.
// A thread is created to read messages from the pipe, enqueue them and signal to the
// main thread that messages are available.
//

#pragma once

#include <extcomp.he>
#include <extfval.he>
#include <hwnd.he>
#include <gdi.he>
#include <map>
#include "MessageQueue.h"

class CefInstance {
public:
	CefInstance(HWND hwnd);
	~CefInstance();
	bool IsHwnd(HWND hwnd) { return hwnd_ == hwnd; }
	qbool CallMethod(EXTCompInfo *eci);
	void PopMessages();
	void Resize();
	
	void AddRef() { ++reference_count_; }
	void SubRef() { if(!--reference_count_) delete this; }

	static UINT PIPE_MESSAGES_AVAILABLE;

	// RefPtr provides a reference-counted pointer to CefInstance.
	struct RefPtr {
		RefPtr(EXTCompInfo *eci, HWND hwnd) {
			instance_ = static_cast<CefInstance*>(ECOfindObject(eci, hwnd));
			if(!instance_)
				throw std::runtime_error("Unknown HWND.");
			instance_->AddRef();
		}
		~RefPtr() {
			instance_->SubRef();
		}
		CefInstance *operator->() const { return instance_; }
	private:
		CefInstance *instance_;
	};

protected:
	static DWORD WINAPI StartPipeListenerThread(LPVOID pVoid);
	DWORD RunPipeListenerThread();
	bool CreatePipe();
	void ClosePipe();
	void ReadMessage();
	void ReadComplete(DWORD bytes_read);
	void GrowReadBuffer();
	void WriteMessage(const std::wstring &name, const std::wstring &argument);
	void WriteMessage(std::wstring message);
	
	void InitWebView();
	void ShutDownWebView();
	void ExecuteJavaScript(std::wstring code) {
		WriteMessage(L"execute", code);
	}
	
	void sendDoShowMessage(const std::string &arg);
	void sendOnConsoleMessageAdded(const std::string &arg);
	void sendOnFrameLoadingFailed(const std::string &arg);
	void sendOnAddressBarChanged(const std::string &arg);
	void sendOnCustomCompAction(const std::string &arg);

	HWND hwnd_;
	HANDLE listener_thread_;
	HANDLE job_;
	LPOVERLAPPED read_lpo_, write_lpo_;
	HANDLE pipe_;
	PSECURITY_ATTRIBUTES pSa_;
	std::string pipe_name_;
	std::wstring read_buffer_;
	std::string::size_type read_offset_;

	bool cef_ready_;
	std::vector<std::wstring> messages_to_write_;
	std::auto_ptr<MessageQueue> message_queue_;
	
	// the command name map allows for an efficient string switch statement.
	enum CommandName {
		ready,
		console,
		address,
		loadError,
		showMsg,
		closeModule,
		gotFocus,
		customEvent
	};
	typedef std::map<std::string, CommandName> CommandNameMap;
	CommandNameMap command_name_map_;
	void InitCommandNameMap();

	// we need to be reference-counted to prevent early deletion.
	int reference_count_;
};

enum Enum {
	// -------Obj methods ------------	
	ofnavigateToUrl = 1000,
	ofHistoryGoBack,
	ofHistoryGoForward,
	ofInitWebView,
	ofFocus,
	ofUnFocus,
	ofShutDownWebView,
	ofCancelDownload,
	ofStartDownload,
	ofGetCompData,
	ofSetCompData,
	ofSendActionToComp,
	ofSendCustomEvent,

	// -------Obj event methods ------------	
	evDoCloseModule = 1100,
	evDoShowMessage,

	// -------Obj event methods for webview listener ------------	
	evOnConsoleMessageAdded = 1110,
	evOnDocumentReady,
	evOnFrameLoadingFailed,
	evOnTitleChange,
	evOnAddressBarChanged,
	evOnOpenNewWindow,
	evOnDownloadRequest,
	evOnDownloadUpdate,
	evOnDownloadFinish,
	evOnJsInitFailed,
	evOnCustomCompAction,
	evOnCompInit,
	evOnGotFocus,


	// -------Static methods ------------	

	// -------Properties	------------	
	pBasePath = 4000,
	pUserPath
};

/*
// Method ids
const qlong 
	// -------Obj Methods ------------	
	ofnavigateToUrl = 1000,
	ofHistoryGoBack = 1001,
	ofHistoryGoForward = 1002,
	ofInitWebView = 1003,
	ofFocus = 1004,
	ofUnFocus = 1005,
	ofShutDownWebView = 1006,
	ofCancelDownload = 1007,
	ofStartDownload = 1008,	
	ofGetCompData = 1009,
	ofSetCompData = 1010,
	ofSendActionToComp = 1014,

	// -------Obj Event Methods ------------	
	evDoCloseModule = 1100,	
	evDoShowMessage = 1102,	
	
	// -------Obj Event der Web View Listener ------------	
	evOnConsoleMessageAdded = 1110,
	evOnDocumentReady = 1111,
	evOnFrameLoadingFailed = 1112,
	evOnTitleChange = 1113,
	evOnAddressBarChanged = 1114,
	evOnOpenNewWindow = 1115,
	evOnDownloadRequest = 1116,
	evOnDownloadUpdate = 1117,
	evOnDownloadFinish = 1118,
	evOnJsInitFailed = 1120,
	evOnCustomCompAction = 1121,
	evOnCompInit = 1122,
	evOnGotFocus = 1123,
	

	// -------Static Methods ------------	

	// -------Properties	------------	
	pBasePath	= 4000,
	pUserPath	= 4001
	
;*/
