test : test.cpp UDPReceiver.cpp UDPSender.cpp
	g++ -std=c++17 test.cpp UDPReceiver.cpp UDPSender.cpp -o test -lpthread