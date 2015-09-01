#include "message_pipe.h"
#include "include/cef_runnable.h"
#include "include/wrapper/cef_helpers.h"
#include <windows.h>
#include <algorithm>
#include <sstream>

void ErrorExit(std::wstring msg, DWORD dw = 0) {
		// Retrieve the system error message for the last-error code
		LPTSTR lpMsgBuf = 0;
		if(!dw)
		dw = GetLastError(); 

		FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, NULL );

		// Display the error message and exit the process
	std::wostringstream ss;
	ss << msg << " failed with error " << dw << " (" << lpMsgBuf << ")";
		MessageBox(NULL, ss.str().c_str(), TEXT("Error"), MB_OK); 

		LocalFree(lpMsgBuf);
		//ExitProcess(dw); 
	DCHECK(false);
}

MessagePipe::MessagePipe(
	CefString pipe_name, 
	PipeOperationHandler *pipe_handler,
	int max_read_attempts,
	std::wstring::size_type start_size)
	:
	pipe_(INVALID_HANDLE_VALUE),
	pipe_name_(pipe_name), 
	pipe_handler_(pipe_handler), 
	max_read_attempts_(max_read_attempts),
	read_buffer_(), 
	read_offset_(0),
	lpo_(NULL) {

	read_buffer_.resize(start_size); // will be dynamically resized.
	
	lpo_ = (LPOVERLAPPED) GlobalAlloc(GPTR, sizeof(OVERLAPPED)); 
	if(lpo_ == NULL)
		throw std::bad_alloc();
	ZeroMemory(lpo_, sizeof(OVERLAPPED));
	lpo_->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	//lpo_->hEvent = this; // hEvent won't be used, so we can store this in it.
}

MessagePipe::~MessagePipe() {
	Disconnect();
	if(lpo_) {
		CloseHandle(lpo_->hEvent);
		GlobalFree(lpo_);
	}
}

void MessagePipeClient::ConnectAttempt(int attempt) {
	/*
	if (!CefCurrentlyOn(TID_IO)) {
		CefPostTask(TID_IO, NewCefRunnableMethod(this, &MessagePipeClient::ConnectAttempt, attempt));
		return;
	}
	*/
	if(pipe_ == INVALID_HANDLE_VALUE && pipe_handler_) {
		pipe_ = CreateFile(pipe_name_.c_str(),
			GENERIC_READ | GENERIC_WRITE, 
			0,    // sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			FILE_FLAG_OVERLAPPED,
			NULL);          // no template file 
		if(pipe_ != INVALID_HANDLE_VALUE) {
			DWORD dwMode = PIPE_READMODE_MESSAGE;
			SetNamedPipeHandleState(pipe_, &dwMode, NULL, NULL);
			//if(!SetNamedPipeHandleState(pipe_, &dwMode, NULL, NULL)) 
			//	ErrorExit(L"SetNamedPipeHandleState");
			if(pipe_handler_)
				pipe_handler_->OnConnectCompleted();
			return;
		}
		DWORD err = GetLastError();
		if(err == ERROR_ACCESS_DENIED)
			ErrorExit(L"CreateFile - access denied");
		// there's no server.
		if(max_conn_attempts_ && attempt >= max_conn_attempts_) {
			if(pipe_handler_)
				pipe_handler_->OnConnectFailed(err);
		} else // try again later.
			CefPostDelayedTask(TID_IO, NewCefRunnableMethod(this, &MessagePipeClient::ConnectAttempt, attempt+1), 100);
	}
}

MessagePipeServer::MessagePipeServer(
	CefString pipe_name, 
	PipeOperationHandler *pipe_handler,
	int max_read_attempts,
	std::wstring::size_type start_size) :
MessagePipe(pipe_name, pipe_handler, max_read_attempts, start_size) {
	PSECURITY_DESCRIPTOR pSd = NULL;
	PCWSTR szSDDL = 
		L"D:"
        L"(A;OICI;GA;;;AU)"
        L"(A;OICI;GA;;;BA)";
	if(!ConvertStringSecurityDescriptorToSecurityDescriptor(szSDDL, SDDL_REVISION_1, &pSd, NULL))
		ErrorExit(L"ConvertStringSecurityDescriptorToSecurityDescriptor");
	pSa_ = (PSECURITY_ATTRIBUTES)LocalAlloc(LPTR, sizeof(*pSa_));
    if(pSa_ == NULL)
		ErrorExit(L"LocalAlloc");
    pSa_->nLength = sizeof(*pSa_);
    pSa_->lpSecurityDescriptor = pSd;
    pSa_->bInheritHandle = FALSE;
}

MessagePipeServer::~MessagePipeServer() {
    if(pSa_) {
        if(pSa_->lpSecurityDescriptor)
            LocalFree(pSa_->lpSecurityDescriptor);
        LocalFree(pSa_);
    }
	pSa_ = NULL;
}

bool MessagePipeServer::Connect(int attempts, int delay) {
	DCHECK(pipe_ == INVALID_HANDLE_VALUE);

	pipe_ = CreateNamedPipe(pipe_name_.c_str(), 
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_REJECT_REMOTE_CLIENTS,
		1, 0, 0, 
		NMPWAIT_USE_DEFAULT_WAIT, 
		pSa_);
	if(pipe_ == INVALID_HANDLE_VALUE) {
		if(pipe_handler_)
			pipe_handler_->OnConnectFailed(GetLastError());
		return false;
	}
	if(pipe_handler_)
		pipe_handler_->OnConnectCompleted();
	return true;
}

void MessagePipe::Disconnect() {
	/*
	if (!CefCurrentlyOn(TID_IO)) {
		CefPostTask(TID_IO, NewCefRunnableMethod(this, &MessagePipe::Disconnect));
		return;
	}*/
	CloseHandle(pipe_);
	pipe_ = INVALID_HANDLE_VALUE;
}

void MessagePipe::ResetPipeHandler(PipeOperationHandler *pipe_handler) {
	/*if (!CefCurrentlyOn(TID_IO)) {
		CefPostTask(TID_IO, NewCefRunnableMethod(this, &MessagePipe::ResetPipeHandler, pipe_handler));
		return;
	}*/
	pipe_handler_ = pipe_handler;
}

void MessagePipe::ReadAttempt(int attempt) {
	if (!CefCurrentlyOn(TID_IO)) {
		CefPostTask(TID_IO, NewCefRunnableMethod(this, &MessagePipe::ReadAttempt, attempt));
		return;
	}
	DWORD cbBytesRead;
	lpo_->Offset = 0;
	lpo_->OffsetHigh = 0;
	if(!ReadFile( 
		pipe_, 
		(LPVOID) &read_buffer_[read_offset_], 
		(read_buffer_.size() - read_offset_) * sizeof(std::wstring::value_type),
		&cbBytesRead,
		lpo_)) {
		DWORD err = GetLastError();
		if(err == ERROR_IO_PENDING) {
			// a read is pending.
			AwaitReadCompletion(lpo_);
		} else if(err == ERROR_MORE_DATA) {
			GrowReadBuffer();
			QueueRead();
		} else if(err == ERROR_PIPE_LISTENING) {
			// there's no client.
			if(max_read_attempts_ && attempt >= max_read_attempts_) {
				if(pipe_handler_)
					pipe_handler_->OnReadFailed(err);
			} else // try again later.
				CefPostDelayedTask(TID_IO, NewCefRunnableMethod(this, &MessagePipe::ReadAttempt, attempt+1), 100);
		} else if(err == ERROR_BROKEN_PIPE || err == ERROR_INVALID_HANDLE) {
			// the pipe was closed.
			if(pipe_handler_)
				pipe_handler_->OnReadFailed(err);
		} else
			ErrorExit(L"ReadFile");
	} else {
		read_offset_ += cbBytesRead / sizeof(std::wstring::value_type);
		if(read_offset_ >= read_buffer_.size())
			GrowReadBuffer();
		ReadComplete();
	}
}

void MessagePipe::AwaitReadCompletion(LPOVERLAPPED lpo) {
	CEF_REQUIRE_IO_THREAD();
	switch(WaitForSingleObject(lpo->hEvent, 0)) {
	case WAIT_OBJECT_0: {
		DWORD cbBytesRead;
		if(GetOverlappedResult(pipe_, lpo, &cbBytesRead, FALSE)) {
			read_offset_ += cbBytesRead / sizeof(std::wstring::value_type);
			if(read_offset_ >= read_buffer_.size())
				GrowReadBuffer();
			ReadComplete();
		} else {
			DWORD err = GetLastError();
			if(err == ERROR_MORE_DATA) {
				GrowReadBuffer();
				QueueRead();
			} else {
				if(pipe_handler_)
					pipe_handler_->OnReadFailed(err);
			}
		}
		return;
	}
	case WAIT_TIMEOUT:
		CefPostDelayedTask(TID_IO, NewCefRunnableMethod(this, &MessagePipe::AwaitReadCompletion, lpo), 100);
		return;
	}
}

void MessagePipe::ReadComplete() {
	std::wstring::size_type i = read_buffer_.find((wchar_t) 0);
	while(i != std::wstring::npos && i < read_offset_) {
		//read_buffer_.resize(i);
		if(pipe_handler_)
			pipe_handler_->OnReadCompleted(read_buffer_);
		std::wstring::size_type sz = read_buffer_.size();
		read_buffer_.erase(0, i+1);
		read_buffer_.resize(sz);
		read_offset_ -= i+1;
		i = read_buffer_.find((wchar_t) 0);
	}
	QueueRead();
}

void MessagePipe::GrowReadBuffer() {
	// double the buffer size and read more.
	std::wstring::size_type sz = read_buffer_.size();
	read_buffer_.resize(2*sz);
	read_offset_ = sz;
	//QueueRead();
}

CefRefPtr<CefProcessMessage> MessagePipe::PipeOperationHandler::ToProcessMessage(std::wstring &buffer) {
	std::wstring::size_type i = buffer.find(':');
	if(i != std::wstring::npos)
		buffer[i] = 0;
	CefString message_name = buffer.data();
	CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(message_name);
	if(i != std::wstring::npos) {
		// there was an argument following a : character.
		message->GetArgumentList()->SetString(0, buffer.data() + i + 1);
	}
	return message;
}

void MessagePipe::QueueWrite(const CefString &name, const CefString &message, bool synchronous) {
	if(!synchronous && !CefCurrentlyOn(TID_IO)) {
		CefPostTask(TID_IO, NewCefRunnableMethod(this, &MessagePipe::QueueWrite, name, message, synchronous));
		return;
	}
	LPOVERLAPPED write_lpo = (LPOVERLAPPED) GlobalAlloc(GPTR, sizeof(OVERLAPPED)); 
	if(write_lpo == NULL)
		throw std::bad_alloc();
	ZeroMemory(write_lpo, sizeof(OVERLAPPED));
	std::wstring *write_buffer = new std::wstring(name);
	write_buffer->append(L":");
	write_buffer->append(message);
	write_buffer->append(1, L'\0'); // null terminator.
	write_lpo->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	int bytes = write_buffer->size() * sizeof(std::wstring::value_type);
	DWORD written;
	if(!WriteFile(
		pipe_, 
		write_buffer->data(), 
		bytes,
		&written, write_lpo)) {
		DWORD err = GetLastError();
		if(err == ERROR_IO_PENDING) {
			// a write is pending.
			if(synchronous) {
				if(WaitForSingleObject(write_lpo->hEvent, INFINITE) != WAIT_OBJECT_0)
					ErrorExit(L"WaitForSingleObject");
				WriteComplete(write_lpo, write_buffer);
			} else
				AwaitWriteCompletion(write_lpo, write_buffer);
		} else if(pipe_handler_)
			pipe_handler_->OnWriteFailed(err);
	} else {
		WriteComplete(write_lpo, write_buffer);
	}
}

void MessagePipe::Write(const CefString &name, const CefString &message) {
	QueueWrite(name, message, true);
} 

void MessagePipe::AwaitWriteCompletion(LPOVERLAPPED write_lpo, std::wstring *write_buffer) {
	CEF_REQUIRE_IO_THREAD();
	switch(WaitForSingleObject(write_lpo->hEvent, 0)) {
	case WAIT_OBJECT_0:
		WriteComplete(write_lpo, write_buffer);
		return;
	case WAIT_TIMEOUT:
		CefPostDelayedTask(TID_IO, NewCefRunnableMethod(this, &MessagePipe::AwaitWriteCompletion, write_lpo, write_buffer), 100);
		return;
		// other cases... WAIT_FAILED, WAIT_ABANDONED?
	}
}

void MessagePipe::WriteComplete(LPOVERLAPPED write_lpo, std::wstring *write_buffer) {
	if(pipe_handler_)
		pipe_handler_->OnWriteCompleted(*write_buffer);
	delete write_buffer;
	CloseHandle(write_lpo->hEvent);
	GlobalFree(write_lpo);
}