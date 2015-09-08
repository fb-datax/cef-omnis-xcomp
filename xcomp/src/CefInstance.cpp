#include "CefInstance.h"
#include "Win32Error.h"
#include "OmnisTools.h"
#include <sstream>
#include <sddl.h>
#include <atlbase.h>
#include <atlconv.h>
#include "rapidjson\document.h"

using namespace OmnisTools;

typedef rapidjson::Document JSONDocument;

UINT CefInstance::PIPE_MESSAGES_AVAILABLE = 0;

CefInstance::CefInstance(HWND hwnd) :
	hwnd_(hwnd),
	listener_thread_(INVALID_HANDLE_VALUE),
	job_(NULL),
	pipe_(INVALID_HANDLE_VALUE),
	read_offset_(0),
	cef_ready_(false),
	message_queue_(new MessageQueue()),
	reference_count_(0)
{
	InitCommandNameMap();

	// create a custom windows event for signalling from the pipe listener 
	// thread to the main thread.
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

	read_buffer_.resize(1024); // will be dynamically resized if necessary.

	read_lpo_ = (LPOVERLAPPED) GlobalAlloc(GPTR, sizeof(OVERLAPPED)); 
	if(read_lpo_ == NULL)
		throw std::bad_alloc();
	ZeroMemory(read_lpo_, sizeof(OVERLAPPED));
	read_lpo_->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	write_lpo_ = (LPOVERLAPPED) GlobalAlloc(GPTR, sizeof(OVERLAPPED)); 
	if(write_lpo_ == NULL)
		throw std::bad_alloc();
	ZeroMemory(write_lpo_, sizeof(OVERLAPPED));
	write_lpo_->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
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
}

void CefInstance::InitWebView() {
	if(listener_thread_ != INVALID_HANDLE_VALUE || pipe_ != INVALID_HANDLE_VALUE || job_)
		throw std::runtime_error("WebView already initialized.");

	if(!CreatePipe())
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
		<< " --parent-hwnd=" << std::dec << (int) hwnd_
		<< " --disable-web-security"
		<< " --enable-experimental-web-platform-features"
		<< " --allow-file-access-from-files"
		<< " --allow-universal-access-from-files"
		<< " --url=file:///" // this is needed to allow navigation to file urls.
		<< " --pipe-name=" << pipe_name_;
	job_ = CreateJobObject(NULL, NULL);
	if(!job_)
		throw Win32Error();
	// create a job so that CEF will be killed if Omnis is killed unexpectedly.
	// normally ShutDownWebView will be called and the browser will be closed gracefully.
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if(!SetInformationJobObject(job_, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
		throw Win32Error();
	STARTUPINFO startup_info;
	ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
	PROCESS_INFORMATION process_info;
    ZeroMemory(&process_info, sizeof(process_info));
	if (!CreateProcess(exe_path.c_str(), &cmd_line.str()[0], NULL, NULL, FALSE, 
		CREATE_NO_WINDOW | CREATE_BREAKAWAY_FROM_JOB, 
		NULL, NULL, &startup_info, &process_info))
		throw Win32Error();
	if(!AssignProcessToJobObject(job_, process_info.hProcess))
		throw Win32Error();
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

	// wait for the CEF exe to connect to the pipe.
	read_lpo_->Offset = 0;
	read_lpo_->OffsetHigh = 0;
	if(!ConnectNamedPipe(pipe_, read_lpo_)) {
		DWORD err = GetLastError();
		if(err == ERROR_IO_PENDING) {
			if(WaitForSingleObject(read_lpo_->hEvent, INFINITE) != WAIT_OBJECT_0)
				throw Win32Error();
		} else if(err != ERROR_PIPE_CONNECTED)
			throw Win32Error();
	} else
		throw std::exception(); // should never happen for overlapped calls.

	// wait for the first "ready" message from CEF.
	//ReadMessage();
	//std::auto_ptr<MessageQueue::Message> message(message_queue_->Pop());
	//if(message->value_ != "ready:")
	//	throw std::runtime_error("Expecting ready message");

	// start the pipe listener thread.
	listener_thread_ = CreateThread(NULL, 0, CefInstance::StartPipeListenerThread, this, 0, NULL);
	if(!listener_thread_)
		throw Win32Error();
}

void CefInstance::ShutDownWebView() {
	// shutting down. we tell CEF to close all browsers, close the pipe and wait for
	// the listener thread to exit.
	if(listener_thread_ == INVALID_HANDLE_VALUE || pipe_ == INVALID_HANDLE_VALUE)
		throw std::runtime_error("WebView not initialized.");
	WriteMessage(L"exit", L"");
	ClosePipe();
	if(WaitForSingleObject(listener_thread_, 5000) != WAIT_OBJECT_0)
		throw Win32Error();
	CloseHandle(listener_thread_);
	listener_thread_ = NULL;
	if(job_) {
		CloseHandle(job_);
		job_ = NULL;
	}
}

void CefInstance::sendDoShowMessage(const std::string &arg) {
	// the argument should be an array in JSON format.
	JSONDocument doc;
	doc.Parse(arg.c_str());
	if(doc.IsArray()) {
		std::auto_ptr<EXTCompInfo> eci(new EXTCompInfo());
		eci->mParamFirst = 0;
		for(int i=0; i<doc.Size(); ++i) {
			if(doc[i].IsString()) {
				EXTfldval val;
				GetEXTFldValFromString(val, doc[i].GetString());
				ECOaddParam(eci.get(), &val, 0, 0, 0, i+1, 0);
			}
		}
		ECOsendCompEvent(hwnd_, eci.get(), evDoShowMessage, qtrue);
		ECOmemoryDeletion(eci.get());
	} else
		TraceLog(TARGET_NAME ": Bad showMsg message.");
}

void CefInstance::sendOnConsoleMessageAdded(const std::string &arg) {
	// the argument should be an object in JSON format.
	JSONDocument doc;
	doc.Parse(arg.c_str());
	if(!doc.HasParseError() && doc.IsObject()) {
		std::auto_ptr<EXTCompInfo> eci(new EXTCompInfo());
		eci->mParamFirst = 0;
		EXTfldval message;
		if(doc.HasMember("message"))
			GetEXTFldValFromString(message, doc["message"].GetString());
		else
			GetEXTFldValFromString(message, "");
		ECOaddParam(eci.get(), &message, 0, 0, 0, 1, 0);
		EXTfldval line;
		GetEXTFldValFromInt(line, doc["line"].GetInt());
		ECOaddParam(eci.get(), &line, 0, 0, 0, 2, 0);
		if(doc.HasMember("source")) {
			EXTfldval source;
			GetEXTFldValFromString(source, doc["source"].GetString());
			ECOaddParam(eci.get(), &source, 0, 0, 0, 3, 0);
		}
		ECOsendCompEvent(hwnd_, eci.get(), evOnConsoleMessageAdded, qtrue); 
		ECOmemoryDeletion(eci.get()); 
	} else
		TraceLog(TARGET_NAME ": Bad console message.");
}

void CefInstance::sendOnFrameLoadingFailed(const std::string &arg) {
	// the argument should be an object in JSON format.
	JSONDocument doc;
	doc.Parse(arg.c_str());
	if(!doc.HasParseError() && doc.IsObject()) {
		std::auto_ptr<EXTCompInfo> eci(new EXTCompInfo());
		eci->mParamFirst = 0;
		EXTfldval error_code;
		if(doc.HasMember("errorCode"))
			GetEXTFldValFromInt(error_code, doc["errorCode"].GetInt());
		else
			GetEXTFldValFromInt(error_code, 0);
		ECOaddParam(eci.get(), &error_code, 0, 0, 0, 1, 0);
		EXTfldval error_text;
		if(doc.HasMember("errorText"))
			GetEXTFldValFromString(error_text, doc["errorText"].GetString());
		else
			GetEXTFldValFromString(error_text, "");
		ECOaddParam(eci.get(), &error_text, 0, 0, 0, 2, 0);
		EXTfldval failed_url;
		if(doc.HasMember("failedUrl"))
			GetEXTFldValFromString(failed_url, doc["failedUrl"].GetString());
		else
			GetEXTFldValFromString(failed_url, "");
		ECOaddParam(eci.get(), &failed_url, 0, 0, 0, 3, 0);
		ECOsendCompEvent(hwnd_, eci.get(), evOnFrameLoadingFailed, qtrue); 
		ECOmemoryDeletion(eci.get()); 
	} else
		TraceLog(TARGET_NAME ": Bad loadError message.");
}

void CefInstance::sendOnAddressBarChanged(const std::string &arg) {
	std::auto_ptr<EXTCompInfo> eci(new EXTCompInfo());
	eci->mParamFirst = 0;
	EXTfldval url;
	GetEXTFldValFromString(url, arg.c_str());
	ECOaddParam(eci.get(), &url, 0, 0, 0, 1, 0);
	ECOsendCompEvent(hwnd_, eci.get(), evOnAddressBarChanged, qtrue); 
	ECOmemoryDeletion(eci.get()); 
}

CefInstance::~CefInstance() {
	if(read_lpo_) {
		CloseHandle(read_lpo_->hEvent);
		GlobalFree(read_lpo_);
	}
	if(write_lpo_) {
		CloseHandle(write_lpo_->hEvent);
		GlobalFree(write_lpo_);
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
	while(true)
		ReadMessage();
	return 0;
}

void CefInstance::ReadMessage() {
	// make a synchronous read from the pipe and push the resulting message to
	// the queue.
	int attempt = 1;
	while(true) {
		// make a blocking read from the named pipe.
		DWORD bytes_read;
		read_lpo_->Offset = 0;
		read_lpo_->OffsetHigh = 0;
		if(!ReadFile( 
			pipe_, 
			(LPVOID) &read_buffer_[read_offset_], 
			(read_buffer_.size() - read_offset_) * sizeof(std::wstring::value_type),
			&bytes_read,
			read_lpo_)) {
			DWORD err = GetLastError();
			if(err == ERROR_IO_PENDING) {
				// wait for the read to complete.
				switch(WaitForSingleObject(read_lpo_->hEvent, INFINITE)) {
					case WAIT_OBJECT_0: {
						attempt = 1;
						if(GetOverlappedResult(pipe_, read_lpo_, &bytes_read, FALSE)) {
							read_offset_ += bytes_read / sizeof(std::wstring::value_type);
							if(read_offset_ >= read_buffer_.size())
								GrowReadBuffer();
							return ReadComplete(bytes_read);
						} else {
							DWORD err = GetLastError();
							if(err == ERROR_MORE_DATA) {
								GrowReadBuffer();
							} else if(err == ERROR_BROKEN_PIPE) {
								// the main thread closed the pipe. we're done.
								ExitThread(0);
							} else {
								throw Win32Error(err);
							}
						}
						continue;
					}
					default:
						throw Win32Error();
				}
			} else if(err == ERROR_MORE_DATA) {
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
				// the pipe was closed. we're done.
				ExitThread(0);
			} else
				throw Win32Error();
		} else {
			// the read completed immediately.
			attempt = 1;
			return ReadComplete(bytes_read);
		}
	}
}

void CefInstance::ReadComplete(DWORD bytes_read) {
	read_offset_ += bytes_read / sizeof(std::wstring::value_type);
	if(read_offset_ >= read_buffer_.size())
		GrowReadBuffer();
	// convert the wstring message to to string for 
	// non-unicode xcomp.
	std::string message(CW2A(read_buffer_.c_str()));
	read_offset_ = 0;

	// add a message to the queue and signal the main thread.
	message_queue_->Push(new MessageQueue::Message(message));
	PostMessage(hwnd_, PIPE_MESSAGES_AVAILABLE, 0, 0);
}

void CefInstance::GrowReadBuffer() {
	// double the buffer size.
	std::wstring::size_type sz = read_buffer_.size();
	read_buffer_.resize(2*sz);
	read_offset_ = sz;
}

void CefInstance::WriteMessage(const std::wstring &name, const std::wstring &argument) {
	std::wstring message(name);
	message.append(L":");
	message.append(argument);
	WriteMessage(message);
}

void CefInstance::WriteMessage(std::wstring message) {
	if(!cef_ready_) {
		// defer writing this message until CEF is ready.
		messages_to_write_.push_back(message);
		return;
	}
	if(pipe_ == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Pipe is closed.");
	message.append(1, L'\0'); // null terminator.
	int bytes = message.size() * sizeof(std::wstring::value_type);
	DWORD written;
	write_lpo_->Offset = 0;
	write_lpo_->OffsetHigh = 0;
	if(!WriteFile(
		pipe_, 
		message.data(), 
		bytes,
		&written, write_lpo_)) {
		DWORD err = GetLastError();
		if(err == ERROR_IO_PENDING) {
			if(WaitForSingleObject(write_lpo_->hEvent, INFINITE) != WAIT_OBJECT_0)
				throw Win32Error();
		} else
			throw Win32Error();
	}
}

bool CefInstance::CreatePipe() {
	if(pipe_ != INVALID_HANDLE_VALUE)
		throw std::runtime_error("Pipe is already connected");

	pipe_ = CreateNamedPipe(pipe_name_.c_str(), 
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_REJECT_REMOTE_CLIENTS,
		1, 0, 0, 
		NMPWAIT_USE_DEFAULT_WAIT, 
		pSa_);
	if(pipe_ == INVALID_HANDLE_VALUE) {
		return false;
	}
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
			InitWebView();
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			break;
		}

		case ofShutDownWebView: {
			ShutDownWebView();
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			break;
		}

		case ofnavigateToUrl: {
			EXTParamInfo* paramInfo = ECOfindParamNum(eci,1);
			if (paramInfo) {
				rtnCode = qtrue;
				hasRtnVal = qtrue;	
				EXTfldval fval( (qfldval)paramInfo->mData);
				std::string url_a = OmnisTools::GetStringFromEXTFldVal(fval);
				std::wstring url = CA2W(url_a.c_str());
				WriteMessage(L"navigate", url);
				//std::wstringstream code;
				//code << "window.location.href='" << url << "';";
				//ExecuteJavaScript(code.str());
			}
			break;
		}
		
		case ofCancelDownload: {
			EXTParamInfo* paramInfo = ECOfindParamNum(eci,1);
			if (paramInfo) {
				rtnCode = qtrue;
				hasRtnVal = qtrue;	
				EXTfldval fval( (qfldval)paramInfo->mData);
				int downloadId = OmnisTools::GetIntFromEXTFldVal(fval);
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
			int downloadId = OmnisTools::GetIntFromEXTFldVal(fvalDownloadId);

			EXTfldval fvalPath((qfldval)pPathParam->mData);
			std::string path = OmnisTools::GetStringFromEXTFldVal(fvalPath);


			// ### rtnVal.setLong(WebBrowser::startDownload(downloadId,path));
			
			break;
		}

		case ofHistoryGoBack: {
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			ExecuteJavaScript(L"window.history.back();");
			break;
		}

		case ofHistoryGoForward: {
			rtnCode = qtrue;
			hasRtnVal = qtrue;
			ExecuteJavaScript(L"window.history.forward();");
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
				std::string url = OmnisTools::GetStringFromEXTFldVal(fval);
				// ### std::string result = WebBrowser::getDataFromComp(url);
				// ### OmnisTools::GetEXTFldValFromString(rtnVal,result);
			}
			break;
		}

		case ofSetCompData: {
			EXTParamInfo* pCompId = ECOfindParamNum(eci,1);
			EXTParamInfo* pData = ECOfindParamNum(eci,2);
			if (pCompId) {
				EXTfldval fvalComp((qfldval)pCompId->mData);
				EXTfldval fvalData((qfldval)pData->mData);
				std::string comp = OmnisTools::GetStringFromEXTFldVal(fvalComp);
				std::string data = OmnisTools::GetStringFromEXTFldVal(fvalData);
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
				std::string compId = OmnisTools::GetStringFromEXTFldVal(fvalCompId);

				EXTParamInfo* pType = ECOfindParamNum(eci,2);
				EXTfldval fvalType ((qfldval)pType->mData);
				std::string type = OmnisTools::GetStringFromEXTFldVal(fvalType);

				EXTParamInfo* pParam1 = ECOfindParamNum(eci,3);
				EXTfldval fvalParam1 ((qfldval)pParam1->mData);
				std::string param1 = OmnisTools::GetStringFromEXTFldVal(fvalParam1);

				EXTParamInfo* pParam2 = ECOfindParamNum(eci,4);
				EXTfldval fvalParam2 ((qfldval)pParam2->mData);
				std::string param2 = OmnisTools::GetStringFromEXTFldVal(fvalParam2);

				EXTParamInfo* pParam3 = ECOfindParamNum(eci,5);
				EXTfldval fvalParam3 ((qfldval)pParam3->mData);
				std::string param3 = OmnisTools::GetStringFromEXTFldVal(fvalParam3);

				EXTParamInfo* pParam4 = ECOfindParamNum(eci,6);
				EXTfldval fvalParam4 ((qfldval)pParam4->mData);
				std::string param4 = OmnisTools::GetStringFromEXTFldVal(fvalParam4);

				EXTParamInfo* pParam5 = ECOfindParamNum(eci,7);
				EXTfldval fvalParam5 ((qfldval)pParam5->mData);
				std::string param5 = OmnisTools::GetStringFromEXTFldVal(fvalParam5);

				EXTParamInfo* pParam6 = ECOfindParamNum(eci,8);
				EXTfldval fvalParam6 ((qfldval)pParam6->mData);
				std::string param6 = OmnisTools::GetStringFromEXTFldVal(fvalParam6);

				EXTParamInfo* pParam7 = ECOfindParamNum(eci,9);
				EXTfldval fvalParam7 ((qfldval)pParam7->mData);
				std::string param7 = OmnisTools::GetStringFromEXTFldVal(fvalParam7);

				EXTParamInfo* pParam8 = ECOfindParamNum(eci,10);
				EXTfldval fvalParam8 ((qfldval)pParam8->mData);
				std::string param8 = OmnisTools::GetStringFromEXTFldVal(fvalParam8);

				EXTParamInfo* pParam9 = ECOfindParamNum(eci,11);
				EXTfldval fvalParam9 ((qfldval)pParam9->mData);
				std::string param9 = OmnisTools::GetStringFromEXTFldVal(fvalParam9);
				
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

void CefInstance::InitCommandNameMap() {
	command_name_map_["ready"] = ready;
	command_name_map_["console"] = console;
	command_name_map_["address"] = address;
	command_name_map_["loadError"] = loadError;
	command_name_map_["showMsg"] = showMsg;
	command_name_map_["closeModule"] = closeModule;
	command_name_map_["gotFocus"] = gotFocus;
}

void CefInstance::PopMessages() {
	MessageQueue::Message *message;
	while(message = message_queue_->Pop()) {
		std::auto_ptr<MessageQueue::Message> m(message);

		// split the message value on the first colon.
		std::string::size_type i = m->value_.find(':');
		std::string arg;
		if(i != std::string::npos) {
			m->value_[i] = 0;
			++i;
			if(i < m->value_.size())
				arg = &m->value_[i];
		}
		std::string message_name = m->value_.c_str();
		CommandNameMap::const_iterator command = command_name_map_.find(message_name);
		if(command != command_name_map_.end()) {
			switch(command->second) {
				case ready: {
					TraceLog("CEF ready.");
					cef_ready_ = true;
					std::vector<std::wstring>::const_iterator i;
					for(i = messages_to_write_.begin(); i != messages_to_write_.end(); ++i)
						WriteMessage(*i);
					messages_to_write_.clear();
					break;
				}
				case console: {
					sendOnConsoleMessageAdded(arg);
					break;
				}
				case address: {
					sendOnAddressBarChanged(arg);
					break;
				}
				case loadError: {
					sendOnFrameLoadingFailed(arg);
					break;
				}
				case showMsg: {
					sendDoShowMessage(arg);
					break;
				}
				case closeModule: {
					ECOsendEvent(hwnd_, evDoCloseModule);
					break;
				}
				case gotFocus: {
					HWND frame = hwnd_;
					//SetFocus(frame);
					//SendMessage(frame, WM_PARENTNOTIFY, WM_LBUTTONDOWN, 0);
					frame = GetParent(frame);
					frame = GetParent(frame);
					frame = GetParent(frame);
					frame = GetParent(frame); // <-- this is the bordered window within Omnis
					HWND omnis = GetParent(frame);
					ECOsendEvent(hwnd_, evOnGotFocus);
					
					//SetWindowPos(frame, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					//SetWindowPos(frame, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					
					//PostMessage(frame, WM_LBUTTONDOWN, MK_LBUTTON, 0x00050005);
					//PostMessage(frame, WM_LBUTTONUP, MK_LBUTTON, 0x00050005);

					// ### How do we activate frame since it's a child of Omnis? ###
					//SendMessage(frame, WM_PARENTNOTIFY, WM_LBUTTONDOWN, (1211 << 16) | 8);
					//SendMessage(frame, WM_SYSKEYDOWN, 0x12, 0x20380001);

					//SendMessage(frame, WM_MOUSEACTIVATE, (WPARAM) omnis, (513 << 16) | 2);
					/*WINDOWPOS pos = {0};
					pos.hwnd = frame;
					pos.hwndInsertAfter = HWND_TOP;
					pos.flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;
					SendMessage(frame, WM_WINDOWPOSCHANGING, 0, (LPARAM) &pos);*/
					//ECOsendEvent(frame, ECE_FORMTOTOP);
					//HWND active = (HWND) SendMessage(omnis, WM_MDIGETACTIVE, 0, 0);
					//SendMessage(active, WM_MDIACTIVATE, (WPARAM) frame, 0);
					//SendMessage(omnis, WM_SETFOCUS, (WPARAM) frame, 0);
					//SendMessage(omnis, WM_KILLFOCUS, (WPARAM) frame, 0);

					//SetWindowPos(frame, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					//SetActiveWindow(frame);
					//SetForegroundWindow(frame);
					//SetFocus(frame);
					//SetWindowPos(frame, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					//WNDsetWindowPos(frame, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					//WNDbringWindowToTop(frame);
					//WNDshowWindow(frame, SW_HIDE);
					//WNDshowWindow(frame, SW_SHOW);
					break;
				}
			}
		} else {
			std::stringstream ss;
			ss << "Unknown CEF message: " << message_name;
			if(!arg.empty())
				ss << ":" << arg;
			TraceLog(ss.str());
		}
	}
}

void CefInstance::Resize() {
	if(pipe_ != INVALID_HANDLE_VALUE)
		WriteMessage(L"resize");
}