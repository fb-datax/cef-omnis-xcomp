#include "CefInstance.h"
#include "Win32Error.h"
#include "OmnisTools.h"
#include <sstream>
#include <sddl.h>

UINT CefInstance::PIPE_MESSAGES_AVAILABLE = 0;

CefInstance::CefInstance(HWND hwnd) :
	hwnd_(hwnd),
	listner_thread_(INVALID_HANDLE_VALUE),
	pipe_(INVALID_HANDLE_VALUE),
	read_offset_(0),
	message_queue_(new MessageQueue())
{
	// for efficient execution of commands from the pipe, we populate a map.
	command_name_map_["ready"] = ready;

	// create a custom windows event for signalling from the
	// pipe listener thread to the main thread.
	if(!PIPE_MESSAGES_AVAILABLE) {
		PIPE_MESSAGES_AVAILABLE = RegisterWindowMessage("Pipe messages available");
		if(!PIPE_MESSAGES_AVAILABLE)
			throw Win32Error();
	}

	// the pipe name is constructed to be unique to the given hwnd. 
	// for example: \\.\pipe\CefWebLib_434972
	// TARGET_NAME is provided as a preprocessor directive in project properties.
	std::stringstream ss;
	ss << "\\\\.\\pipe\\" TARGET_NAME "_" << hwnd;
	pipe_name_ = ss.str();

	read_buffer_.resize(1024); // will be dynamically resized.
	lpo_ = (LPOVERLAPPED) GlobalAlloc(GPTR, sizeof(OVERLAPPED)); 
	if(lpo_ == NULL)
		throw std::bad_alloc();
	ZeroMemory(lpo_, sizeof(OVERLAPPED));
	lpo_->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	PSECURITY_DESCRIPTOR pSd = NULL;
	PCSTR szSDDL = 
		"D:"
        "(A;OICI;GA;;;AU)"
        "(A;OICI;GA;;;BA)";
	if(!ConvertStringSecurityDescriptorToSecurityDescriptor(szSDDL, SDDL_REVISION_1, &pSd, NULL))
		throw Win32Error();
		//throw std::runtime_error("ConvertStringSecurityDescriptorToSecurityDescriptor");
	pSa_ = (PSECURITY_ATTRIBUTES)LocalAlloc(LPTR, sizeof(*pSa_));
    if(pSa_ == NULL)
		throw std::bad_alloc();
    pSa_->nLength = sizeof(*pSa_);
    pSa_->lpSecurityDescriptor = pSd;
    pSa_->bInheritHandle = FALSE;

	if(!CreatePipe())
		throw Win32Error();

	// start the pipe listener thread.
	listner_thread_ = CreateThread(NULL, 0, CefInstance::StartPipeListenerThread, this, 0, NULL);
	if(!listner_thread_)
		throw Win32Error();

	// find the full CefWebLib executable path based on the path of the CefWebLib dll.
	std::string dll_name = TARGET_NAME ".dll";
	HMODULE h_dll = LoadLibrary(dll_name.c_str());
	std::string exe_path;
	exe_path.resize(MAX_PATH);
	DWORD size = GetModuleFileName(h_dll, &exe_path[0], MAX_PATH);
	if(!size)
		throw Win32Error();
	// truncate the ".dll" and add the relative path to the executable.
	exe_path.resize(size-4);
	exe_path += "\\" TARGET_NAME ".exe";

	// spawn the CEF exe with command line args
	std::stringstream cmd_line;
	cmd_line << '"' << exe_path << "\""
		<< " --parent-hwnd=" << std::dec << (int) hwnd
		<< " --url=" // ####
		<< " --pipe-name=" << pipe_name_;
	STARTUPINFO startup_info;
	ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
	PROCESS_INFORMATION process_info;
    ZeroMemory(&process_info, sizeof(process_info));
	if(!CreateProcess(exe_path.c_str(), &cmd_line.str()[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startup_info, &process_info))
		throw Win32Error();
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
}

CefInstance::~CefInstance() {
	ClosePipe();
	if(lpo_) {
		CloseHandle(lpo_->hEvent);
		GlobalFree(lpo_);
	}
    if(pSa_) {
        if(pSa_->lpSecurityDescriptor)
            LocalFree(pSa_->lpSecurityDescriptor);
        LocalFree(pSa_);
    }
}

DWORD WINAPI CefInstance::StartPipeListenerThread(LPVOID pVoid) {
	CefInstance *self = static_cast<CefInstance*>(pVoid);
	return self->RunPipeListenerThread();
}

DWORD CefInstance::RunPipeListenerThread() {
	int attempt = 1;
	while(true) {
		// make a blocking read from the named pipe.
		DWORD bytes_read;
		if(!ReadFile( 
			pipe_, 
			(LPVOID) &read_buffer_[read_offset_], 
			(read_buffer_.size() - read_offset_) * sizeof(std::wstring::value_type),
			&bytes_read,
			NULL)) {
			DWORD err = GetLastError();
			if(err == ERROR_MORE_DATA) {
				GrowReadBuffer();
			} else if(err == ERROR_PIPE_LISTENING) {
				// there's no client yet.
				if(attempt++ >= 30) {
					// the client failed to connect. give up.
					// ### throw Win32Error(err);
				}
				// wait for the client to connect.
				Sleep(100);
			} else if(err == ERROR_BROKEN_PIPE || err == ERROR_INVALID_HANDLE) {
				// the pipe was closed.
				throw Win32Error(err);
			} else
				throw Win32Error();
		} else {
			// the read has completed successfully.
			attempt = 1;
			read_offset_ += bytes_read / sizeof(std::wstring::value_type);
			if(read_offset_ >= read_buffer_.size())
				GrowReadBuffer();
			// perform a quick-and dirty conversion of wstring to string for 
			// non-unicode xcomp.
			std::string message(read_buffer_.begin(), read_buffer_.end());

			// add a message to the queue and signal the main thread.
			message_queue_->push(new MessageQueue::Message(message));
			PostMessage(hwnd_, PIPE_MESSAGES_AVAILABLE, 0, 0);
		}
	}
	return 0;
}

void CefInstance::GrowReadBuffer() {
	// double the buffer size.
	std::wstring::size_type sz = read_buffer_.size();
	read_buffer_.resize(2*sz);
	read_offset_ = sz;
}

void CefInstance::WriteMessage(std::wstring name, std::wstring argument) {
	if(pipe_ == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Pipe is closed.");
	std::wstring write_buffer(name);
	write_buffer.append(L":");
	write_buffer.append(argument);
	write_buffer.append(1, L'\0'); // null terminator.
	int bytes = write_buffer.size() * sizeof(std::wstring::value_type);
	DWORD written;
	if(!WriteFile(
		pipe_, 
		write_buffer.data(), 
		bytes,
		&written, NULL))
		throw Win32Error();
}

bool CefInstance::CreatePipe() {
	if(pipe_ != INVALID_HANDLE_VALUE)
		throw std::runtime_error("Pipe is already connected");

	pipe_ = CreateNamedPipe(pipe_name_.c_str(), 
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_REJECT_REMOTE_CLIENTS,
		1, 0, 0, 
		NMPWAIT_USE_DEFAULT_WAIT, 
		pSa_);
	if(pipe_ == INVALID_HANDLE_VALUE) {
		//if(pipe_handler_)
		//	pipe_handler_->OnConnectFailed(GetLastError());
		return false;
	}
	//if(pipe_handler_)
	//	pipe_handler_->OnConnectCompleted();
	return true;
}

void CefInstance::ClosePipe() {
	CloseHandle(pipe_);
	pipe_ = INVALID_HANDLE_VALUE;
}

qbool CefInstance::CallMethod(EXTCompInfo *eci) {
	EXTfldval rtnVal; 
	qbool rtnCode		=	qfalse;
	qbool hasRtnVal		=	qfalse;

	switch(ECOgetId(eci)) {
		case ofInitWebView: {
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			// ### rtnVal.setLong(WebBrowser::initWebView());
			break;
		}

		case ofShutDownWebView: {
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			// ### rtnVal.setLong(WebBrowser::shutDownWebView());
			break;
		}

		case ofnavigateToUrl: {
			EXTParamInfo* paramInfo = ECOfindParamNum(eci,1);
			if (paramInfo) {
				rtnCode = qtrue;
				hasRtnVal = qtrue;	
				EXTfldval fval( (qfldval)paramInfo->mData);
				std::string url = OmnisTools::getStringFromEXTFldVal(fval);
				// ### rtnVal.setLong(WebBrowser::navigateToUrl(url));
			}
			break;
		}
		
		case ofCancelDownload: {
			EXTParamInfo* paramInfo = ECOfindParamNum(eci,1);
			if (paramInfo) {
				rtnCode = qtrue;
				hasRtnVal = qtrue;	
				EXTfldval fval( (qfldval)paramInfo->mData);
				int downloadId = OmnisTools::getIntFromEXTFldVal(fval);
				// ### rtnVal.setLong(WebBrowser::cancelDownload(downloadId));
			}
			break;
		}

		case ofStartDownload: {
			EXTParamInfo* pDownloadId = ECOfindParamNum(eci,1);
			EXTParamInfo* pPathParam = ECOfindParamNum(eci,2);
			
			rtnCode = qtrue;
			hasRtnVal = qtrue;	
			
			EXTfldval fvalDownloadId((qfldval)pDownloadId->mData);
			int downloadId = OmnisTools::getIntFromEXTFldVal(fvalDownloadId);

			EXTfldval fvalPath((qfldval)pPathParam->mData);
			std::string path = OmnisTools::getStringFromEXTFldVal(fvalPath);


			// ### rtnVal.setLong(WebBrowser::startDownload(downloadId,path));
			
			break;
		}

		case ofHistoryGoBack: {
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			// ### rtnVal.setLong(WebBrowser::historyBack());
			break;
		}

		case ofHistoryGoForward: {
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			// ### rtnVal.setLong(WebBrowser::historyForward());
			break;
		}
		case ofFocus: {
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			// ### rtnVal.setLong(WebBrowser::focusWebView());
			break;
		}
		case ofUnFocus: {
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			// ### rtnVal.setLong(WebBrowser::unFocusWebView());
			break;
		}
		case ofGetCompData: {
			EXTParamInfo* paramInfo = ECOfindParamNum(eci,1);
			if (paramInfo) {
				rtnCode = qtrue;
				hasRtnVal = qtrue;	
				
				EXTfldval fval( (qfldval)paramInfo->mData);
				std::string url = OmnisTools::getStringFromEXTFldVal(fval);
				// ### std::string result = WebBrowser::getDataFromComp(url);
				// ### OmnisTools::getEXTFldValFromString(rtnVal,result);
			}
			break;
		}

		case ofSetCompData: {
			EXTParamInfo* pCompId = ECOfindParamNum(eci,1);
			EXTParamInfo* pData = ECOfindParamNum(eci,2);
			if (pCompId) {
				EXTfldval fvalComp((qfldval)pCompId->mData);
				EXTfldval fvalData((qfldval)pData->mData);
				std::string comp = OmnisTools::getStringFromEXTFldVal(fvalComp);
				std::string data = OmnisTools::getStringFromEXTFldVal(fvalData);
				// ### rtnVal.setLong(WebBrowser::setDataForComp(comp,data));
				rtnCode = qtrue;
				hasRtnVal = qtrue;	
			}
			break;
		}
		case ofSendActionToComp: {

			EXTParamInfo* pCompId = ECOfindParamNum(eci,1);
			if (pCompId) {
				EXTParamInfo* pCompId = ECOfindParamNum(eci,1);
				EXTfldval fvalCompId ((qfldval)pCompId->mData);
				std::string compId = OmnisTools::getStringFromEXTFldVal(fvalCompId);

				EXTParamInfo* pType = ECOfindParamNum(eci,2);
				EXTfldval fvalType ((qfldval)pType->mData);
				std::string type = OmnisTools::getStringFromEXTFldVal(fvalType);

				EXTParamInfo* pParam1 = ECOfindParamNum(eci,3);
				EXTfldval fvalParam1 ((qfldval)pParam1->mData);
				std::string param1 = OmnisTools::getStringFromEXTFldVal(fvalParam1);

				EXTParamInfo* pParam2 = ECOfindParamNum(eci,4);
				EXTfldval fvalParam2 ((qfldval)pParam2->mData);
				std::string param2 = OmnisTools::getStringFromEXTFldVal(fvalParam2);

				EXTParamInfo* pParam3 = ECOfindParamNum(eci,5);
				EXTfldval fvalParam3 ((qfldval)pParam3->mData);
				std::string param3 = OmnisTools::getStringFromEXTFldVal(fvalParam3);

				EXTParamInfo* pParam4 = ECOfindParamNum(eci,6);
				EXTfldval fvalParam4 ((qfldval)pParam4->mData);
				std::string param4 = OmnisTools::getStringFromEXTFldVal(fvalParam4);

				EXTParamInfo* pParam5 = ECOfindParamNum(eci,7);
				EXTfldval fvalParam5 ((qfldval)pParam5->mData);
				std::string param5 = OmnisTools::getStringFromEXTFldVal(fvalParam5);

				EXTParamInfo* pParam6 = ECOfindParamNum(eci,8);
				EXTfldval fvalParam6 ((qfldval)pParam6->mData);
				std::string param6 = OmnisTools::getStringFromEXTFldVal(fvalParam6);

				EXTParamInfo* pParam7 = ECOfindParamNum(eci,9);
				EXTfldval fvalParam7 ((qfldval)pParam7->mData);
				std::string param7 = OmnisTools::getStringFromEXTFldVal(fvalParam7);

				EXTParamInfo* pParam8 = ECOfindParamNum(eci,10);
				EXTfldval fvalParam8 ((qfldval)pParam8->mData);
				std::string param8 = OmnisTools::getStringFromEXTFldVal(fvalParam8);

				EXTParamInfo* pParam9 = ECOfindParamNum(eci,11);
				EXTfldval fvalParam9 ((qfldval)pParam9->mData);
				std::string param9 = OmnisTools::getStringFromEXTFldVal(fvalParam9);
				
				// ### rtnVal.setLong(WebBrowser::sendActionToComp(compId,type,param1,param2,param3,param4,param5,param6,param7,param8,param9));
				rtnCode = qtrue;
				hasRtnVal = qtrue;	
			}
			break;
		}
	}
	if(hasRtnVal)
		ECOaddParam(eci, &rtnVal);
	return rtnCode;
}

void CefInstance::PopMessages() {
	MessageQueue::Message *message;
	while(message = message_queue_->pop()) {
		std::auto_ptr<MessageQueue::Message> m(message);

		// split the message value on the first colon.
		std::string::size_type i = m->value_.find(':');
		std::string arg;
		if(i != std::string::npos) {
			m->value_[i] = 0;
			arg = &m->value_[i+1];
		}
		std::string command = m->value_.c_str();
		switch(command_name_map_[command]) {
			case ready:
				WriteMessage(L"ping", L"test");
				break;
		}
	}
}