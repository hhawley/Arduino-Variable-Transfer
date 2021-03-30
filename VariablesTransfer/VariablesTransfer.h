#pragma once

#include <Arduino.h>
#include <inttypes.h>
#include <Stream.h>

// You can make these bigger or smaller dependently on your RAM
// restrictions
#ifndef VARIABLE_MAX_BUFFER
	#define VARIABLE_MAX_BUFFER 9
#endif

// The size of the number read buffer
// Increase this depending on the number of digits given your precision
#ifndef NUMBER_MAX_BUFFER
	#define NUMBER_MAX_BUFFER 6
#endif

// Error defines
#define VT_NO_DATA 0 
#define VT_ERR_INDEX_OUT_BOUNDS 1
#define VT_ERR_INCORRECT_FORMAT 2
#define VT_ERR_RW_BIT 3
#define VT_SUCESS 42
// end Error defines

// Change this guy if you want an unique delimiter for any reason
#ifndef VT_DELIMITER
	#define VT_DELIMITER ','
#endif

class VarTransfer {
private:
	static uint16_t** __variables_ptr;
	static char __buffer[NUMBER_MAX_BUFFER];

	/// Some devices do not need this, but it helps with stability.
	static inline char ___read_char(Stream& stream) {
		while(stream.peek() < 0);
		return stream.read();
	}

public:

	static void linkVariable(const byte& index, uint16_t* var);
	static uint8_t processVariables(Stream& stream);

};

