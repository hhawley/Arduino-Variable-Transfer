#include "executer.h"
#include "i2c_comm.h"
#include "serial_comm.h"

#include <unistd.h>

#include <cstdio>
#include <sstream>
#include <climits>
#include <iostream>


Executer::Executer(ChangeVariablesListener* listener) : _listener(listener) { }
Executer::Executer(std::shared_ptr<ChangeVariablesListener>& listener) : _listener(listener) { }

Executer::~Executer() { 
	_outputFile.close();

}

void Executer::init() {
	_outputFile.open("output.txt");
	__load_config();
}

void Executer::run() {

	if(_listener->isNewMessage()) {

			std::string cmd = __parse_command(_listener->getMessage());
			std::string response = __send_command(cmd);
			std::cout << response << std::endl;
	}

}

void Executer::__run_thread() {

	for(int reset  = 0; reset < 5; reset++) {
		try {
			while(true) {

				if(_listener->isNewMessage()) {

						std::string cmd = __parse_command(_listener->getMessage());
						std::string response = __send_command(cmd);
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

void Executer::__load_config() {
	std::ifstream file("data.cfg");

	bool areTheyCommands=false;
	bool areTheyAlias=false;

	std::string line;
	std::string variable, value;
	while(std::getline(file, line)) {
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
				printf("New Command: %s, %s\n", item.Key.c_str(), item.Value.c_str());
				_commandDictionary[item.Key] = item.Value;
			}
		} else if(areTheyAlias) {

			auto item = ___parse_config_line(line);
			auto i2cItem = ___parse_config_line(item.Value, ' ');

			printf("New device: %s, %s\n", item.Key.c_str(), item.Value.c_str());

			if(i2cItem.Value.length() != 0) {
				printf("with i2c address: %s\n", i2cItem.Value.c_str());
				_devicesAlias[item.Key] = i2cItem.Key;
				_devicesAlias[i2cItem.Key] = i2cItem.Key;

				int address = std::stoi(i2cItem.Value);
				_devices[i2cItem.Key] = std::shared_ptr<I2Communicator>(new I2Communicator(address));

				try {
					_devices[i2cItem.Key]->init(i2cItem.Key);
				} catch (std::exception& e) {
					std::cout << "Failed to initialize I2C device. Not using " << item.Value << " A.K.A " << item.Value << std::endl;
					std::cout << e.what() << endl;


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
					std::cout << "Failed to initialize device. Not using " << item.Value << " A.K.A " << item.Value << std::endl;
					std::cout << e.what() << endl;


					//Cleaning up not necessary items...
					_devicesAlias.erase(item.Key);
					_devicesAlias.erase(item.Value);

					_devices.erase(item.Value);
				}
			}

		}

	}
}

std::string Executer::__parse_command(const std::string& msg) {

	std::string parse_msg = "";
	unsigned int pos;

	std::string trim_msg = msg.substr(0, msg.find('\n'));

	std::cout << trim_msg.c_str() << std::endl;
	pos = trim_msg.find("{");
	if(pos != UINT_MAX) {
		//Msg is a raw message
		printf("Nothing to do\n");
	} else {
		//Msg is a command
		pos = trim_msg.find(" ");
		if(pos != UINT_MAX) {
			//Has arguments
			std::string args = trim_msg.substr(pos+1);
			std::string command = trim_msg.substr(0, pos);

			std::vector< std::string > arguments;
			___get_arguments(args, arguments);

			parse_msg = _commandDictionary[command];
			char prse_msg_char[100];
			int error = 0;

			if(arguments.size() == 1) {
				error = sprintf(prse_msg_char, parse_msg.c_str(), arguments[0].c_str());
			} else if(arguments.size() == 2) {
				error = sprintf(prse_msg_char, parse_msg.c_str(), arguments[0].c_str(), arguments[1].c_str());
			} else if(arguments.size() == 3) {
				error = sprintf(prse_msg_char, parse_msg.c_str(), arguments[0].c_str(), arguments[1].c_str(), arguments[2].c_str());
			} else if(arguments.size() > 3) {
				printf("%s\n", "Too many arguments in the command.");
				error = -1;
			}

			if(error < 0) { parse_msg = "";	}
			else { parse_msg = prse_msg_char; }

		} else {

			// No arguments, just look it up in the dictionary
			parse_msg = _commandDictionary[trim_msg];
		}

	}

	return parse_msg;

}

std::string Executer::__send_command(const std::string& cmd) {
	if(cmd == "") {
		return "No command";
	}

	auto checkFormat = cmd.find(" ");

	if(checkFormat == UINT_MAX) {
		return "Incorrect type of command";
	}

	std::string device = cmd.substr(0, checkFormat);
	std::string cmd_toSend = cmd.substr(checkFormat + 1);

	auto r_device = _devicesAlias[device];
	if(r_device == "") {
		return cmd + ": Device does not exist";
	}

	auto pdevice = _devices[r_device];
	if(pdevice) {
		if(___s_msg(pdevice, cmd_toSend)) {

			std::cout << "Sent messsage " << cmd_toSend << " to device " << r_device << std::endl;
			auto r_msg = ___r_msg(pdevice);

			if(r_msg != "") {
				_outputFile << r_msg;
				return "Recieved messsage" + r_msg + "from device " + r_device + "\n";

			} else {

				return "with no response";

			}

		}

		return "Failed to send message to " + r_device;

	} else {
		throw std::runtime_error("Device " + r_device + " does not exist.");
	}
}

void Executer::___get_arguments(const std::string& args, 
std::vector<std::string>& args_v) {

	unsigned int newPos = args.find(" ");
	if(newPos == UINT_MAX) {
		args_v.push_back(args);
	} else {

		std::string argument = args.substr(0, newPos);
		args_v.push_back(argument);

		std::string other_arguments = args.substr(newPos+1);

		___get_arguments(other_arguments, args_v);
	}
}

// Source: https://stackoverflow.com/questions/31103883/reading-key-value-pairs-from-a-file-and-ignoring-comment-lines
DictionaryItem Executer::___parse_config_line(const std::string& line, const char& delimiter) {

	DictionaryItem item;
	std::istringstream is_line(line);

	if(line[0] == '#') { return item; }

	if(std::getline(is_line, item.Key, delimiter)) {
		std::getline(is_line, item.Value);
	}

	return item;

}

bool Executer::___s_msg(std::shared_ptr<Stream>& device, const std::string& msg) {

	for(int retry = 0; retry < RETRIES; retry++) {

		if(device->sendMessage(msg)) {
			return true;
		}
		else { usleep (50); }

	}

	return false;

}

std::string Executer::___r_msg(std::shared_ptr<Stream>& device) {

	for(int retry = 0; retry < RETRIES; retry++) {

		auto msg = device->readMessage();

		if(msg != "") {
			return msg;
		}
		else { usleep (100); }

	}

	return "";

}
