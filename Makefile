SOURCES := $(wildcard *.cpp *.hpp)

test : $(SOURCES)
	g++ -std=c++17 test.cpp UDPReceiver.cpp UDPSender.cpp -o test -lpthread -I.