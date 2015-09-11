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
	qbool SetProperty(EXTCompInfo *eci);
	qbool GetProperty(EXTCompInfo *eci);
	void PopMessages();
	void Resize();
	void Focus();
	
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
	
	void ShowMessage(const std::string &arg);
	void ConsoleMessage(const std::string &arg);
	void SendLoadingStateChange(const std::string &arg);
	void SendLoadEnd(const std::string &arg);
	void SendLoadError(const std::string &arg);
	void SendTitleChange(const std::string &arg);
	void SendAddressChange(const std::string &arg);
	void SendCustomEvent(const std::string &arg);

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
	bool context_menus_;
	std::vector<std::wstring> messages_to_write_;
	std::auto_ptr<MessageQueue> message_queue_;
	
	// the command name map allows for an efficient string switch statement.
	enum CommandName {
		ready,
		console,
		title,
		address,
		loadingStateChange,
		loadStart,
		loadEnd,
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
	ofHistoryBack,
	ofHistoryForward,
	ofCancelDownload,
	ofStartDownload,
	ofSendCustomEvent,

	// ------- Obj event methods ------------	
	evLoadingStateChange = 1100,
	evLoadStart,
	evLoadEnd,
	evLoadError,
	evTitleChange,
	evAddressChange,
	evDownloadRequest,
	evDownloadUpdate,
	evDownloadFinish,
	evCustomEvent,
	evCompInit,
	evGotFocus,
	
	// ------- Static methods ------------	

	// ------- Properties ------------	
	pContextMenus = 4000,
};
