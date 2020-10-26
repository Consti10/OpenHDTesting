Compile code:
g++ -std=c++17 test.cpp UDPReceiver.cpp UDPSender.cpp -o test -lpthread

Collection of commands:
git add . && git commit -m "X" && git push
git reset --hard && git pull

Receive via udp and write to file
nc -u -l 5060 > newfile.txt

Send file via udp
cat newfile.txt | nc -u localhost 5060
nc -u -l 6002 | nc -u "192.168.0.13" 6003

Listen on localhost and forward to ground pi via ethernet
nc -u -l 6002 | nc -u "192.168.0.13" 6003
//
Same but forward via svpcom

// make executable
chmod u+x my_wfb_rx.sh && chmod u+x my_wfb_tx.sh 



