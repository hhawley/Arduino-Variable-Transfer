#pragma once

#include "Arduino.h"
#include <inttypes.h>
#include <Stream.h>

//You can make these bigger or smaller dependently on your RAM
//restrictions
#define VARIABLE_MAX_BUFFER 5
#define NUMBER_MAX_BUFFER 6

class VarTransfer {
private:
	static uint16_t* __variables_ptr;
	static char __buffer[NUMBER_MAX_BUFFER];

	/// Some devices do not need this, but it helps with stability.
	static inline char ___read_char(Stream& stream) {
		while(stream.peek() < 0);
		return stream.read();
	}

public:

	static void linkVariable(const byte& index, uint16_t*& var);
	static bool readVariable(Stream& stream);

};

