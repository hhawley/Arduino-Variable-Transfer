#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <cstdio>

#include "listener.h"


ChangeVariablesListener::ChangeVariablesListener() { }

ChangeVariablesListener::~ChangeVariablesListener() {

	close(_fileID);
	unlink(_FIFOname.c_str());

}

void ChangeVariablesListener::init(const char* fifoName) {

	_FIFOname = fifoName;
	mkfifo(_FIFOname.c_str(), 0666);
	_fileID = open(_FIFOname.c_str(), O_RDONLY | O_NONBLOCK);

	if(_fileID < 0) {

		perror("Failed to create pipe with error: ");
		throw "";

	}

}


void ChangeVariablesListener::run() {

	int err = read(_fileID, _buffer, 1024);
	if(err > 0 && !_newMessage) {

		_newMessage = true;
		std::cout << "New message\n";
	
	} else if(err < 0) {

		int errsv = errno;
		if(errsv != EAGAIN){

			perror("Failed to read command with error");
			throw std::runtime_error("");

		}
		
	}
}

void ChangeVariablesListener::__run_thread() {

	for(int reset  = 0; reset < 5; reset++) {
		try {
			while(true) {
				int err = read(_fileID, _buffer, 1024);
				if(err > 0 && !_newMessage) {

					_newMessage = true;
					std::cout << "New message\n";
				
				} else if(err < 0) {

					perror("Failed to read command with error:");
					throw std::runtime_error("");

				}

			}
		} catch (std::exception& e) {

			std::cout << "Error in the listener: " << e.what() << std::endl;
			
		}
	}

}

std::thread ChangeVariablesListener::spawn() {
	return std::thread(&ChangeVariablesListener::run, this);
}

std::string ChangeVariablesListener::getMessage() {
	_newMessage = false;
	return std::string(_buffer);
}

bool ChangeVariablesListener::isNewMessage() {
	return _newMessage;
}





// int main() {

// 	int fileID;
// 	string myFIFO = "/tmp/tmpFIFO";
// 	char buffer[1024];
// 	char newBuffer[1024];

// 	mkfifo(myFIFO.c_str(), 0666);

// 	fileID = open(myFIFO.c_str(), O_RDONLY);

// 	cout << "Pipe is up" << endl;

// 	while(1) {

// 		if(read(fileID, buffer, 1024) > 0) {
// 			printf("%s\n", buffer);
// 		}

// 	}

// }
