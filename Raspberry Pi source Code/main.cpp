#include <thread>
#include <memory>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include "i2c_comm.h"
#include "listener.h"
#include "executer.h"

int main(int argc, char const *argv[])
{
	int reset = 0;

	std::unique_ptr<Executer> executer;

	for(reset = 0; reset < 5; reset++) {
		try {

			executer.reset(new Executer());
			executer->init("/tmp/tempFIFO");
			break;

		} catch (std::exception& e) {

			std::cout << "Error at initializing\n" << e.what() << std::endl;

		}
	}

	if(reset == 5) {
		return -1;
	}
	//reset = false;

	// do {
		try{
			while(true) {

				executer->run();
				//usleep (100);

			}
		} catch (std::exception& e) {
			std::cout << "Error at running \n" << e.what() << std::endl;
			// reset = true;
		}


	return 0;
}
