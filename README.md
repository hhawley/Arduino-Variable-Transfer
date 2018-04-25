# Arduino-Variable-Transfer

This project contains the software necessary for a portable, and easy to use program that aims to control experiments or control systems for long periods of time using arduino-like microcontrollers, and a Linux machine.

It is divided in two parts:

  * The arduino library named Variable Transfer
  * Linux code that communicates with the arduinos.

## Variable Transfer Library

The variable transfer library aims to be as lightweight as possible with program, and RAM memory as it was intended to be used with RAM intensive arduino codes. Another special bonus function is that it has very basic error handling. 

It has only two functions:
```c++
void linkVariable(const byte& index, uint16_t*& var); 
```
Which links the pointer var to the internal variable pointer `__variables_ptr`, in the position `index`. If the index you are trying to use is higher than the internal pointer buffer, it will return to avoid crashing. If you want to debug the code to find problems with this variable is recommended to delete/comment that line (line 9) of `VariablesTransfer.cpp`.

```c++
uint8_t processVariables(Stream& stream);; 
```
This function is intended to be used inside the loop() (as shown in the example), and it looks for received data from the stream (Wire and Serial streams have been tested), checks the format, process it, and returns or writes the value that the command indicated. The function returns a 1 byte unsigned integer that indicates if the function has successfully read/wrote to a value, an error, or nothing happened.

The commands this function accepts will always follow the next format:
   
	 {position,R/W,number}
  
Where position is the variable position in the array as indicated in `void linkVariable`, R/W can be either the char 'R' (read) or 'W' (write), and number the string representation of a 16-bit unsigned integer (if bigger sizes are needed, they can always be changed in the source code, but `NUMBER_MAX_BUFFER` has to be changed, too, to accept the bigger numbers). 

### Error values
The return values that it throw are the next:
 
Define | Value | Description
---|---|---
VT_NO_DATA | 0 | No data has been received
ERR_INDEX_OUT_BOUNDS | 1 | The received command has a index that is out of bounds. 
ERR_INCORRECT_FORMAT | 2 | The received command is not in the correct format.
ERR_RW_BIT | 3 | The char indicating if its reading or writing is incorrect.
VT_SUCESS | 42 | Successfully read/wrote the variable. 

## Linux Code

This program relies on FIFOs, and internal Linux functions (read(), write(), etc.) to communicate with one or more arduinos to keep the code as simple, and portable as possible. 

### data.cfg
By using a configuration file named `data.cfg`, it is possible to indicate which ports the devices are connected, their alias name, the commands that it can expect. 

It is divided in two parts: Devices, and Commands.

#### Devices
Indicates the program which ports are being used, and gives them an alias, so the user has an easier time telling them apart. They must have the next form:

`[alias]=[port] (i2c_id)`

Where alias is the name given by the user, port is the full path of the port, and if its a I2C device, i2c_id is the id where the device can be found, otherwise it can be left blank.  

#### Commands
They are the commands sent to the arduinos to avoid using the raw format explained in Variable Transfer Library. They have the next form:

`[command name]=[device] {position,R/W,number}`

Where command name is the given name, device is the arduino/teensy/etc that the program will send the command to which can be either be the alias given or the port, and {position,R/W,number} is the command in the same format as in Variable Transfer Library, but with one change where number can be either a number or `%s` (an example can be seen in the `example-data.cfg` file) which indicates that this command number is variable an expects at least 1 input (more on this on `Example Script`. 

### FIFO
We use a FIFO to communicate with the program, so it can be possible to use bash programming language to indicate which commands to send at the time we desire. With this in mind, the program becomes an intermediate between the script/FIFO and the arduino. 

#### Example Script
`example_script.sh` is a bash script that shows with a real experiment how the program works using `example-data.cfg` as the configuration file. It communicates with the FIFO found in `/tmp/tempFIFO`, using the function `send`. All the command sent to the FIFO are the ones found in the configuration file. We will explain the first two commands as they are good examples of how the program works.

The first command which communicates with the devices is `send "resetMCAValues"` on line 30. This will send that string to the FIFO which will be read by the program, interpret it, and send the command `{3,W,1}` to the device connected to the port `/dev/ttyACM0` which will change the variable in the position 3 with the value 1. If there is any response from the device, the program will read it and print it to the console the program is run at. 

The second command, `setFilterVoltage 100`, is a good example of commands that have inputs. After is has been sent to the FIFO, the program will grab the string `{1,W,%s}` and replace `%s` with `100` which will then send to the device. The input number also follows the same rules as the arduino. For example, if the input is a string the device will return the error ERR_INCORRECT_FORMAT.
