#pragma once

#include "include/cef_process_message.h"
#include <sddl.h>

class MessagePipe : public virtual CefBase {
public:
	class PipeOperationHandler {
	public:
		virtual void OnConnectCompleted() {}
		virtual void OnConnectFailed(DWORD err) {}
		virtual void OnReadCompleted(std::wstring &buffer) {}
		virtual void OnReadFailed(DWORD err) {}
		virtual void OnWriteCompleted(std::wstring &buffer) {}
		virtual void OnWriteFailed(DWORD err) {}
	protected:
		CefRefPtr<CefProcessMessage> ToProcessMessage(std::wstring &buffer);
	};
	MessagePipe(
		CefString pipe_name, 
		PipeOperationHandler *pipe_handler,
		int max_read_attempts = 0, // unlimited.
		std::wstring::size_type start_size = 1024);
	~MessagePipe();
	void Disconnect();
	void ResetPipeHandler(PipeOperationHandler *pipe_handler = NULL);
	// CEF must be initialised before calling QueueRead/QueueWrite.
	void QueueRead() { ReadAttempt(1); }
	void QueueWrite(const CefString &name, const CefString &message, bool synchronous = false);
	void Write(const CefString &name, const CefString &message);
	const CefString &GetPipeName() { return pipe_name_; }
protected:
	void ReadAttempt(int attempt);
	LPOVERLAPPED lpo_;
	HANDLE pipe_;
	CefString pipe_name_;
	PipeOperationHandler *pipe_handler_;
	std::wstring read_buffer_;
	std::wstring::size_type read_offset_;
	int max_read_attempts_;
	
	void AwaitReadCompletion(LPOVERLAPPED lpo);
	void AwaitWriteCompletion(LPOVERLAPPED write_lpo, std::wstring *write_buffer);
	void ReadComplete();
	void GrowReadBuffer();
	void WriteComplete(LPOVERLAPPED write_lpo, std::wstring *write_buffer);

	IMPLEMENT_REFCOUNTING(MessagePipe);
};

class MessagePipeClient : public MessagePipe {
public:
	MessagePipeClient(
		CefString pipe_name, 
		PipeOperationHandler *pipe_handler,
		int max_read_attempts = 0, // unlimited
		int max_conn_attempts = 0, // unlimited
		std::wstring::size_type start_size = 1024) :
	max_conn_attempts_(max_conn_attempts),
	MessagePipe(pipe_name, pipe_handler, max_read_attempts, start_size) {}
	void QueueConnect() { ConnectAttempt(1); }
protected:
	void ConnectAttempt(int attempt);
	int max_conn_attempts_;

};

class MessagePipeServer : public MessagePipe {
public:
	MessagePipeServer(
		CefString pipe_name, 
		PipeOperationHandler *pipe_handler,
		int max_read_attempts = 0, // unlimited.
		std::wstring::size_type start_size = 1024);
	~MessagePipeServer();
	bool Connect(int attempts = 25, int delay = 200);
protected:
	PSECURITY_ATTRIBUTES pSa_;
};