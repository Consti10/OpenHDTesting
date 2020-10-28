SOURCES := $(wildcard *.cpp *.hpp)

test : $(SOURCES)
	g++ -std=c++17 test.cpp Helper/UDPReceiver.cpp Helper/UDPSender.cpp -o test -lpthread -I.