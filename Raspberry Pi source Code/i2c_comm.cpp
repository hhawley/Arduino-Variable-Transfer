#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>
#include <string>
#include <exception>

#include "i2c_comm.h"


I2Communicator::I2Communicator(const int& address) : _i2cAddress(address) {}
I2Communicator::~I2Communicator() {

	close(_fileID);

}

void I2Communicator::init(const std::string& ttydir__o) {
	_fileID = open(ttydir__o.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(_fileID < 0) {
        int errsv = errno;
        std::string errorString = "";
        switch(errsv) {
        case EACCES:
            errorString = "The requested access to the file is not allowed, or search permission is denied";
            break;

        case EFAULT:
            errorString = "The address points outside your accessible address space";
            break;

        case EFBIG:
        case EOVERFLOW:
            errorString = "File is too big";
            break;

        case EINTR:	
            errorString = "While blocked waiting to complete an open of a slow device the call was interrupted by a signal handler";
            break;

        case EMFILE:
            errorString = "The per-process limit on the number of open file descriptors has been reached";
            break;
    
        case ENAMETOOLONG:
            errorString = "Path to device was too long";
            break;

        case ENFILE:
            errorString = "The system-wide limit on the total number of open files has been reached.";
            break;

        case ENODEV:
        case ENXIO:
            errorString = "Device path refers to a device special file and no correspoding device exists.";
            break;

        case ENOENT:
            errorString = "Device path refers to a nonexistent directory";
            break;

        case ENOMEM:
            errorString = "Insufficient kernel memory was available";
            break;

        case ENOTDIR:
            errorString = "A component used as a directory in pathname is not, in fact, a directory";
            break;

        case EPERM:
            errorString = "The operation was prevented by a file seal";
            break;

        case EROFS:
            errorString = "Device path refers to a afile on a read-only filesystem and write was requested";
            break;

        default:
            errorString = "Unknow error";
            break;
    	}

		throw std::runtime_error("Failed to open I2C port: " + ttydir__o + " with error: " + errorString);
	}

	if (ioctl(_fileID, I2C_SLAVE, _i2cAddress) < 0) {
    		int errsv = errno;
    		std::string errorString = "";

    	switch(errsv) {
    	case EBADF:
    		errorString = "FileID is not a valid file descriptor";
    		break;

    	case EFAULT:
    		errorString = "I2C address is inaccessible";
    		break;

    	case EINVAL:
    		errorString = "Internal or external address is not valid";
    		break;

    	case ENOTTY:
    		errorString = "FileID is not associated with a character special device or the specified request does not apply to the kind of object that the file descriptor FileID references";
    		break;

    	default:
    		errorString = "Unknown error";
    		break;

    	}

        	throw std::runtime_error("Unable to get bus access to talk to slave " + std::to_string(_i2cAddress) + "with error: " + errorString);
	}

        if(fcntl(_fileID, F_SETFL, O_NONBLOCK) < 0) {

            throw std::runtime_error("Unable to set port as non-blocking");

        }
}


bool I2Communicator::sendMessage(const std::string& msg__o) {

	std::cout << "Sending message: " <<  msg__o.c_str() << " to address " _i2cAddress << std::endl;
	int status = write(_fileID, (unsigned char*)msg__o.c_str(), msg__o.size());

	if(status < 0) {
        int errsv = errno;
        if(errsv == EAGAIN){
            return "";
        } else {
		  perror(" ");
		  throw std::runtime_error("Failed to send message to address: " + std::to_string(_i2cAddress));
        }
	}

    return true;
}

std::string I2Communicator::readMessage() {

    unsigned char* buff =  new unsigned char[32];
	int status = read(_fileID, buff, sizeof(buff));

    if(status < 0) {

        int errsv = errno;
        if(errsv == EAGAIN){
            return "";
        } else {
            perror(" ");
            throw std::runtime_error("Failed to read message from address: " + std::to_string(_i2cAddress));
        }
    }

    return std::string((char*)buff);

}