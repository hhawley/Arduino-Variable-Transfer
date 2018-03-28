#pragma once

#include "Arduino.h"
#include <inttypes.h>
#include <Stream.h>

//You can make these bigger or smaller dependently on your RAM
//restrictions
#ifndef VARIABLE_MAX_BUFFER
	#define VARIABLE_MAX_BUFFER 7
#endif

#ifndef NUMBER_MAX_BUFFER
	#define NUMBER_MAX_BUFFER 6
#endif

// Error defines
#define VT_NO_DATA 0 
#define ERR_INDEX_OUT_BOUNDS 1
#define ERR_INCORRECT_FORMAT 2
#define ERR_RW_BIT 3
#define VT_SUCESS 42
// end Error defines

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
	static uint8_t processVariables(Stream& stream);

};

