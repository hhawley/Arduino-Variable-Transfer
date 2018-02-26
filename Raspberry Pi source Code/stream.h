#pragma once

#include <string>

class Stream {

public:
	virtual void init(const std::string& ttydir) = 0;
	virtual bool sendMessage(const std::string& msg) = 0;
	virtual std::string readMessage() = 0;

};