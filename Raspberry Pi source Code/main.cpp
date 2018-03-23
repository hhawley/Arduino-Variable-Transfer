#include <thread>
#include <memory>
#include <cstdio>
#include <iostream>
#include "i2c_comm.h"
#include "listener.h"
#include "executer.h"

int main(int argc, char const *argv[])
{
	int reset = 0;
	std::shared_ptr<ChangeVariablesListener> listener;
	std::unique_ptr<Executer> executer;

	for(reset  = 0; reset < 5; reset++) {
		try {

			listener.reset(new ChangeVariablesListener());
			executer.reset(new Executer(listener));

			listener->init("/tmp/tempFIFO");
			executer->init();
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
	// 	try{
			while(true) {

				listener->run();
				executer->run();
				usleep (100);

			}
	// 	} catch (std::exception& e) {
	// 		std::cout << "Error at running \n" << e.what() << std::endl;
	// 		reset = true;
	// 	}

	// } while (reset);

	// std::thread listenerThread;
	// std::thread executerThread;

	// for(reset  = 0; reset < 5; reset++) {
	// 	try {
	// 		std::cout << "Creating threads..." << std::endl;
	// 		listenerThread = listener->spawn();
	// 		executerThread = executer->spawn();
	// 		break;

	// 	} catch (std::exception& e) {

	// 		std::cout << "Error at creating threads " << e.what() << std::endl;
	// 		reset = true;

	// 	}
	// }

	// if(reset == 5) { return -1; }

	// listenerThread.join();
	// executerThread.join();

	return 0;
}
