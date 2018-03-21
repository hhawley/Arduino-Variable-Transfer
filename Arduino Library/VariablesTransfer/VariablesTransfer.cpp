#include <VariablesTransfer.h>

uint16_t* VarTransfer::__variables_ptr = new uint16_t[VARIABLE_MAX_BUFFER];
char VarTransfer::__buffer[NUMBER_MAX_BUFFER] = {0};

// Make sure index is not bigger than VARIABLE_MAX_BUFFER - 1
void VarTransfer::linkVariable(const byte& index, uint16_t*& var) {

	var = __variables_ptr + index;

}

/// read values then traduce them to a variable from I2C or Serial
/// Data is expected to be the next form:
/// {[INDEX],[WRITE TO INDEX=0, READ INDEX = 1],[DATA]}
/// Where:
/// [INDEX]: is an unsigned byte going from 0 to 9
/// [DATA]: is an unsigned 16 bit integer, does not matter if reading
/// Function returns vt_sucess if a sucessfull variable write/read has been made
/// otherwise it returns an error or VT_NO_DATA if nothing happened
uint8_t VarTransfer::processVariables(Stream& stream) {
	if(stream.available() > 0) {
		int8_t latestByte = 0;
		uint8_t index = 0;


		latestByte = ___read_char(stream);
		// Read the stream of data until we find an '{'
		if (latestByte == '{') {

			// I have tried to initialize the buffer here but some devices dont initialize to 0
			// so thats why I made it static and I set all the values to 0 using memset. 
			// I could just do
			// char buffer[NUMBER_MAX_BUFFER] = {0};
			// but I rather it stay as static for now.
			memset(__buffer, 0, sizeof(__buffer));

			latestByte = ___read_char(stream);

			// Read the index for the variable
			index = latestByte - 48;
			if(index > VARIABLE_MAX_BUFFER - 1) { return ERR_INDEX_OUT_BOUNDS; }

			latestByte = ___read_char(stream);
			if(latestByte != ',') { return ERR_INCORRECT_FORMAT; }
			

			latestByte = ___read_char(stream);
			latestByte -= 48;

			if(latestByte == 0) {

				latestByte = ___read_char(stream);
				if(latestByte != ',') { return ERR_INCORRECT_FORMAT; }
				// Start reading for the value of the variable
				for(int i = 0; i < 6; i++) {
					latestByte = ___read_char(stream);
					if(latestByte == '}') { break; }

					__buffer[i] = latestByte;
				}

				cli();
				__variables_ptr[index] = atoi(__buffer);
				sei();
				
				stream.println("OK");
				return VT_SUCESS;

			} else if(latestByte == 1) {

				stream.println(__variables_ptr[index]);
				return VT_SUCESS;

			} else { return ERR_RW_BIT; }
		
			

		}
	} else { return VT_NO_DATA; }

	return VT_NO_DATA;

}
