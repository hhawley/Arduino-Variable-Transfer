#include "executer.h"
#include "i2c_comm.h"
#include "serial_comm.h"

#include <unistd.h>

#include <cstdio>
#include <sstream>
#include <climits>
#include <iostream>


Executer::Executer() : _listener(new FIFOListener()) {}
Executer::Executer(FIFOListener* listener) 
	: _listener(std::make_unique<FIFOListener>(listener)) { }

Executer::~Executer() { 
	_outputFile.close();
}

void Executer::init(const std::string& fifoName__o) {
	_listener->init(fifoName__o);
	_outputFile.open("output.txt");
	__load_config();
}

void Executer::run() {

	_listener->run();
	if(_listener->isNewMessage()) {
			std::string cmd = __parse_command(_listener->getMessage());
			std::string response = __handle_command(cmd);
			std::cout << response << std::endl;
	}

}

void Executer::__run_thread() {

	for(int reset  = 0; reset < 5; reset++) {
		try {
			while(true) {

				if(_listener->isNewMessage()) {

						std::string cmd = __parse_command(_listener->getMessage());
						std::string response = __handle_command(cmd);
						std::cout << response << std::endl;

				}

			}
		} catch (std::exception& e) {

			std::cout << "Error in the executer: " << e.what() << std::endl;

		}
	}

}

std::thread Executer::spawn() {
	return std::thread(&Executer::__run_thread, this);
}

// Loads the alias commands and the devices from data.cfg
void Executer::__load_config() {
	std::ifstream configFile("data.cfg");

	if(!configFile.is_open()) {
		throw  std::runtime_error("Failed to open data.cfg, check if there is an available data.cfg");
	}

	bool areTheyCommands = false;
	bool areTheyAlias = false;

	std::string line;
	std::string variable, value;
	while(std::getline(configFile, line)) {
		if(line.compare("Commands") == 0) {
			areTheyCommands = true;
			areTheyAlias = false;
			continue;

		} else if(line.compare("Devices") == 0) {
			areTheyCommands = false;
			areTheyAlias = true;
			continue;

		} else if(line.length() == 0) { continue; }

		if(areTheyCommands) {

			auto item = ___parse_config_line(line);

			if(item.Key.length() != 0) {
				std::cout << "New Command:" << item.Key.c_str() << ", " << item.Value.c_str() << std::endl;
				_commandDictionary[item.Key] = item.Value;
			}

		} else if(areTheyAlias) {

			auto item = ___parse_config_line(line);
			auto i2cItem = ___parse_config_line(item.Value, ' ');

			std::cout << "New device: " << item.Key.c_str() << ", " << item.Value.c_str() << std::endl;

			if(i2cItem.Value.length() != 0) {
				std::cout << "with i2c address: " << i2cItem.Value.c_str() << std::endl;
				_devicesAlias[item.Key] = i2cItem.Key;
				_devicesAlias[i2cItem.Key] = i2cItem.Key;

				int address = std::stoi(i2cItem.Value);
				_devices[i2cItem.Key] = std::shared_ptr<I2Communicator>(new I2Communicator(address));

				try {
					_devices[i2cItem.Key]->init(i2cItem.Key);
				} catch (std::exception& e) {
					std::cout << "Failed to initialize I2C device. Not using " << item.Value << " A.K.A " << item.Key << std::endl;
					std::cout << e.what() << std::endl;

					//Cleaning up not necessary items...
					_devicesAlias.erase(item.Key);
					_devicesAlias.erase(item.Value);

					_devices.erase(item.Value);
				}

			} else {

				_devicesAlias[item.Key] = item.Value;
				_devicesAlias[item.Value] = item.Value;
				_devices[item.Value] = std::shared_ptr<SerialCommunicator>(new SerialCommunicator());

				try {
					_devices[item.Value]->init(item.Value);
				} catch (std::exception& e) {
					std::cout << "Failed to initialize device. Not using " << item.Value << " A.K.A " << item.Key << std::endl;
					std::cout << e.what() << std::endl;

					//Cleaning up not necessary items...
					_devicesAlias.erase(item.Key);
					_devicesAlias.erase(item.Value);

					_devices.erase(item.Value);
				}
			}

		}

	}

	configFile.close();
}

// Parses a command, finds if its a raw command (i.e {0,0,1}) or an alias command and turns it into a raw command.
std::string Executer::__parse_command(const std::string& msg__o) {

	std::string parseMsg = "";
	unsigned int pos;

	std::string trimMsg = msg__o.substr(0, msg__o.find('\n'));

	std::cout << trimMsg.c_str() << std::endl;
	pos = trimMsg.find("{");
	if(pos != UINT_MAX) {
		//Msg is a raw message
		return msg__o;
	} else {
		//Msg is a command
		pos = trimMsg.find(" ");
		if(pos != UINT_MAX) {
			//Has arguments
			std::string args = trimMsg.substr(pos+1);
			std::string command = trimMsg.substr(0, pos);

			std::vector< std::string > arguments;
			___get_arguments(args, arguments);

			parseMsg = _commandDictionary[command];
			char prse_msg_char[100];
			int error = 0;

			if(arguments.size() == 1) {
				error = sprintf(prse_msg_char, parseMsg.c_str(), arguments[0].c_str());
			} else if(arguments.size() == 2) {
				error = sprintf(prse_msg_char, parseMsg.c_str(), arguments[0].c_str(), arguments[1].c_str());
			} else if(arguments.size() == 3) {
				error = sprintf(prse_msg_char, parseMsg.c_str(), arguments[0].c_str(), arguments[1].c_str(), arguments[2].c_str());
			} else if(arguments.size() > 3) {
				std::cout << "Too many arguments in the command." << std::endl;
				error = -1;
			}

			if(error < 0) { parseMsg = "";	}
			else { parseMsg = prse_msg_char; }

		} else {

			// No arguments, just look it up in the dictionary
			parseMsg = _commandDictionary[trimMsg];
		}

	}

	return parseMsg;

}


// Prepares the command to be sent, checks for errors, and receives the data from the port.
std::string Executer::__handle_command(const std::string& cmd__o) {
	if(cmd__o == "") {
		return "No command";
	}

	auto checkFormat = cmd__o.find(" ");

	if(checkFormat == UINT_MAX) {
		return "Incorrect type of command";
	}

	std::string device = cmd__o.substr(0, checkFormat);
	std::string cmdToSend = cmd__o.substr(checkFormat + 1);

	auto selectedDevice = _devicesAlias[device];
	if(selectedDevice == "") {
		return cmd__o + ": Device does not exist";
	}

	auto pdevice = _devices[selectedDevice];
	if(pdevice) {
		if(___s_msg(pdevice, cmdToSend)) {

			std::cout << "Sent messsage " << cmdToSend << " to device " << selectedDevice << std::endl;
			auto r_msg = ___r_msg(pdevice);

			if(r_msg != "") {
				_outputFile << r_msg;
				return "Recieved messsage: " + r_msg + " from device " + selectedDevice;

			} else {

				return "with no response";

			}

		}

		return "Failed to send message to " + selectedDevice;

	} else {
		throw std::runtime_error("Device " + selectedDevice + " does not exist.");
	}
}

// Gets arguments from a command recursively, and adds them to args_v
void Executer::___get_arguments(const std::string& args, 
std::vector<std::string>& args_v) {

	unsigned int newPos = args.find(" ");
	if(newPos == UINT_MAX) {
		args_v.push_back(args);
	} else {

		std::string argument = args.substr(0, newPos);
		args_v.push_back(argument);

		std::string otherArguments = args.substr(newPos+1);

		___get_arguments(otherArguments, args_v);
	}
}

// Source: https://stackoverflow.com/questions/31103883/reading-key-value-pairs-from-a-file-and-ignoring-comment-lines
DictionaryItem Executer::___parse_config_line(const std::string& line__o, const char& delimiter__o) {

	DictionaryItem item;
	std::istringstream isLine(line__o);

	if(line__o[0] == '#') { return item; }

	if(std::getline(isLine, item.Key, delimiter__o)) {
		std::getline(isLine, item.Value);
	}

	return item;

}

// Send a message to port/device
bool Executer::___s_msg(std::shared_ptr<Stream>& device__o, const std::string& msg__o) {

	for(int retry = 0; retry < RETRIES; retry++) {

		if(device__o->sendMessage(msg__o)) {
			return true;
		}
		else { usleep (50); }

	}

	return false;

}

// Read a message from port/device
std::string Executer::___r_msg(std::shared_ptr<Stream>& device__o) {

	for(int retry = 0; retry < RETRIES; retry++) {

		auto msg = device__o->readMessage();

		if(msg != "") {
			return msg;
		}
		else { usleep (100); }

	}

	return "";

}
