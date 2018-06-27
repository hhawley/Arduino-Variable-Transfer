#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <cstdio>

#include "listener.h"


FIFOListener::FIFOListener() { }

FIFOListener::~FIFOListener() {

	close(_fileID);
	unlink(_FIFOname.c_str());

}

void FIFOListener::init(const std::string& fifoName__o) {

	_FIFOname = fifoName__o;
	mkfifo(_FIFOname.c_str(), 0666);
	_fileID = open(_FIFOname.c_str(), O_RDONLY);

	if(_fileID < 0) {

		int errsv = errno;
		throw std::runtime_error("Failed to create FIFO with error " + errsv);

	}

}


void FIFOListener::run() {

	int err = read(_fileID, _buffer, MAX_BUFFER);
	if(err > 0 && !_newMessage) {

		_newMessage = true;
		std::cout << "New message" << std::endl;
	
	} else if(err < 0) {

		int errsv = errno;
		if(errsv != EAGAIN){

			throw std::runtime_error("Failed to read FIFO with error " + errsv);

		}
		
	}
}

std::string FIFOListener::getMessage() {
	_newMessage = false;
	return std::string(_buffer);
}

bool FIFOListener::isNewMessage() {
	return _newMessage;
}
