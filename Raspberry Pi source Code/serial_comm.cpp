#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <string>
#include <exception>

#include "serial_comm.h" 

SerialCommunicator::SerialCommunicator() {}
SerialCommunicator::~SerialCommunicator() {

	close(_fileID);

}

void SerialCommunicator::init(const std::string& ttydir) {

	_ttydir = ttydir;
	_fileID = open(ttydir.c_str(), O_RDWR | O_NOCTTY );
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

		throw std::runtime_error("Failed to open Serial port: " + _ttydir + " with error: " + errorString);
	}

	// if(fcntl(_fileID, F_SETFL, O_NONBLOCK) < 0) {

 //                throw std::runtime_error("Unable to set port as non-blocking");

 //    }

	__set_interface_attribs(B9600, 0);
	__set_blocking(1);

}

bool SerialCommunicator::sendMessage(const std::string& msg) {

	int status = write(_fileID, msg.c_str(), msg.length());
	usleep ((msg.length()) * 100); 
	printf("Sending message: %s to address %s\n ", msg.c_str(), _ttydir.c_str());

	if(status <  0) {
		int errsv = errno;
        if(errsv == EAGAIN){
            return "";
        } else {
		  perror("Error on serial");
		  throw std::runtime_error("Failed to send message to Serial port: " + _ttydir);
        }
	}

	return true;

}

std::string SerialCommunicator::readMessage() {

	char buff[2048];
	int status = read(_fileID, buff, sizeof(buff));

	if(status < 0) {

		int errsv = errno;
		if(errsv == EAGAIN){
			return "";
		} else {
    		perror("Error on serial");
    		throw std::invalid_argument("Failed to read message to Serial port: " + _ttydir);
        }
	}

	return std::string(buff);
}

//https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
void SerialCommunicator::__set_interface_attribs (int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (_fileID, &tty) != 0)
        {
                perror("error from tcgetattr");
                throw std::runtime_error("Failed to set interface attributes on Serial port");
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 3;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (_fileID, TCSANOW, &tty) != 0)
        {
                perror("error from tcsetattr");
                throw std::runtime_error("Failed to set interface attributes on Serial port");
        }

}

//https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
void SerialCommunicator::__set_blocking (int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (_fileID, &tty) != 0)
        {
                perror ("error from tggetattr");
                throw std::runtime_error("Failed to set interface attributes on Serial port");
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (_fileID, TCSANOW, &tty) != 0) {
                perror("error %d setting term attributes");
                throw std::runtime_error("Failed to set interface attributes on Serial port");
        }
}

