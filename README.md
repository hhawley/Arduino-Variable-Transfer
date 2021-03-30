# Arduino-Variable-Transfer

This project contains the software necessary for a portable, and easy to use program that aims to control experiments or control systems for long periods of time using arduino-like microcontrollers, and a Linux machine.

It is divided in two parts:

  * The arduino library named Variable Transfer
  * Linux code that communicates with the arduinos.

## Variable Transfer Library

The variable transfer library aims to be as lightweight as possible with program, and RAM memory as it was intended to be used with RAM intensive arduino codes. Another special bonus function is that it has very basic error handling. 

It has only two functions:
```c++
void linkVariable(const byte& index, uint16_t* var); 
```
Which links the pointer var to the internal variable pointer `__variables_ptr`, in the position `index`. If the index you are trying to use is higher than the internal pointer buffer, it will return to avoid crashing. If you want to debug the code to find problems with this variable is recommended to delete/comment that line (line 9) of `VariablesTransfer.cpp`.

```c++
uint8_t processVariables(Stream& stream);; 
```
This function is intended to be used inside the loop() (as shown in the example), and it looks for received data from the stream (Wire and Serial streams have been tested), checks the format, process it, and returns or writes the value that the command indicated. The function returns a 1 byte unsigned integer that indicates if the function has successfully read/wrote to a value, an error, or nothing happened.

The commands this function accepts will always follow the next format:
   
	 {Index,R/W,Val}
  
Where position is the variable position in the array as indicated in `void linkVariable`, R/W can be either the char 'R' (read) or 'W' (write), and number the string representation of a 16-bit unsigned integer (if bigger sizes are needed, they can always be changed in the source code, but `NUMBER_MAX_BUFFER` has to be changed, too, to accept the bigger numbers). 

R can be r or 0. W can be w or 1. All characters.

### Error values
The return values that it throw are the next:
 
Define | Value | Description
---|---|---
VT_NO_DATA | 0 | No data has been received
VT_ERR_INDEX_OUT_BOUNDS | 1 | The received command has a index that is out of bounds. 
VT_ERR_INCORRECT_FORMAT | 2 | The received command is not in the correct format.
VT_ERR_RW_BIT | 3 | The char indicating if its reading or writing is incorrect.
VT_SUCESS | 42 | Successfully read/wrote the variable. 

