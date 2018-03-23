#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <fstream>

#include "listener.h"
#include "stream.h"

#define RETRIES 5

typedef std::map< std::string, std::string > Dictionary;
typedef std::map< std::string, std::shared_ptr<Stream> > Devices;
typedef std::ofstream OutputFile;

struct DictionaryItem {
	std::string Key;
	std::string Value;
};


class Executer {
public:

	Executer(ChangeVariablesListener* listener);
	Executer(std::shared_ptr<ChangeVariablesListener>& listener);
	~Executer();

	void init();
	void run();
	std::thread spawn();

private:
	std::shared_ptr<ChangeVariablesListener> _listener;
	OutputFile _outputFile;

	Dictionary _devicesAlias;
	Devices _devices;

	Dictionary _commandDictionary;

	void __load_config();
	void __run_thread();
	std::string __parse_command(const std::string&);
	std::string __handle_command(const std::string&);

	void ___get_arguments(const std::string&, std::vector<std::string>&);
	DictionaryItem ___parse_config_line(const std::string& line__o, const char& delimiter__o = '=');
	bool ___s_msg(std::shared_ptr<Stream>&, const std::string&);
	std::string ___r_msg(std::shared_ptr<Stream>&);
};

