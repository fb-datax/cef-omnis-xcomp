//
// MessageQueue is a thread-safe queue for message passing from the
// named-pipe listener thread to the main thread.
//

#pragma once

#include <windows.h>
#include <string.h>
#include <queue>

class MessageQueue {
public:
	struct Message {
		Message(std::string value) : value_(value) {}
		std::string value_;
	};
	MessageQueue() {
		mutex_ = CreateMutex(NULL, FALSE, NULL);
		if(!mutex_)
			throw std::bad_alloc();
	}
	~MessageQueue() {
		if(mutex_) {
			CloseHandle(mutex_);
			mutex_ = NULL;
		}
	}
	bool push(Message *message) {
		if(WaitForSingleObject(mutex_, INFINITE) == WAIT_OBJECT_0) {
			queue_.push(message);
			if(!ReleaseMutex(mutex_))
				throw std::runtime_error("ReleaseMutex");
			return true;
		}
		// the mutex wait was abandoned.
		return false;
	}
	Message *pop() {
		Message *result = NULL;
		if(WaitForSingleObject(mutex_, INFINITE) == WAIT_OBJECT_0) {
			if(!queue_.empty()) {
				result = queue_.front();
				queue_.pop();
			}
			if(!ReleaseMutex(mutex_))
				throw std::runtime_error("ReleaseMutex");
		}
		// else the mutex wait was abandoned, return NULL.
		return result;
	}
private:
	HANDLE mutex_;
	std::queue<Message*> queue_;
};