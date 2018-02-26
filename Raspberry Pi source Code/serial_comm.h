#pragma once

#include "stream.h"

class SerialCommunicator : public Stream {
public:
	SerialCommunicator();
	~SerialCommunicator();

	virtual void init(const std::string& ttydir);
	virtual bool sendMessage(const std::string& msg);
	virtual std::string readMessage();

private:

	void __set_interface_attribs (int speed, int parity);
	void __set_blocking (int should_block);
	int _fileID;
	std::string _ttydir;

};
