rm ./bin/test
g++ main.cpp i2c_comm.cpp serial_comm.cpp listener.cpp executer.cpp -o bin/test -Wall -std=gnu++11 -O2 -lpthread

