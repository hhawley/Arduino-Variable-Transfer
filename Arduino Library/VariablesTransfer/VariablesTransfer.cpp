#include <VariablesTransfer.h>
#include <Wire.h>

uint16_t* VarTransfer::__variables_ptr = new uint16_t[VARIABLE_MAX_BUFFER];
char VarTransfer::__buffer[NUMBER_MAX_BUFFER] = {0};

// Make sure index is not bigger than VARIABLE_MAX_BUFFER - 1
void VarTransfer::linkVariable(const byte& index, uint16_t*& var) {

	var = __variables_ptr + index;

}

/// read values then traduce them to a variable from I2C or Serial
/// Data is expected to be the next form:
/// {[INDEX],[DATA]}
/// Where:
/// [INDEX]: is an unsigned byte going from 0 to 9
/// [DATA]: is an unsigned 16 bit integer
/// Function returns true if a sucessfull variable update has been made
/// otherwise it returns 0
bool VarTransfer::readVariable(Stream& stream) {
	while(stream.available() > 0) {
		int8_t latestByte = 0;
		uint8_t index = 0;

		// I have tried to initialize the buffer here but some devices dont initialize to 0
		// so thats why I made it static and I set all the values to 0 using memset. 
		// I could just do
		// char buffer[NUMBER_MAX_BUFFER] = {0};
		// but I rather it stay as static for now.
		memset(__buffer, 0, sizeof(__buffer));

		latestByte = ___read_char(stream);
		// Read the stream of data until we find an '{'
		if (latestByte == '{') {

			latestByte = ___read_char(stream);

			// Read the index for the variable
			index = latestByte - 48;
			if(index > VARIABLE_MAX_BUFFER - 1) { return false; }

			latestByte = ___read_char(stream);
			if(latestByte == ',') {
				// Start reading for the value of the variable
				for(int i = 0; i < 6; i++) {
					latestByte = ___read_char(stream);
					if(latestByte == '}') { break; }

					__buffer[i] = latestByte;
				}

				cli();
				__variables_ptr[index] = atoi(__buffer);
				sei();
				return true;
			} else { return false; }
		}
	}

	return false;

}
