#pragma once

#define MAX_BUFFER 1024

#include <string>
#include <thread>

class ChangeVariablesListener {
	
public:
	ChangeVariablesListener();
	~ChangeVariablesListener();

	void init(const char* fifoName);
	void run();
	std::thread spawn();
	std::string getMessage();
	bool isNewMessage();

private:

	void __run_thread();
	
	int _fileID;
	char _buffer[1024];
	std::string _FIFOname;

	bool _isMessageLast;
	bool _isMessage;
	bool _newMessage;

};