#pragma once

#define MAX_BUFFER 1024

#include <string>
#include <thread>

class FIFOListener {
	
public:
	FIFOListener();
	~FIFOListener();

	void init(const std::string&);
	void run();
	std::string getMessage();
	bool isNewMessage();

private:
	
	int _fileID;
	char _buffer[MAX_BUFFER];
	std::string _FIFOname;

	bool _isMessageLast;
	bool _isMessage;
	bool _newMessage;

};