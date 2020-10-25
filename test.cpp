#include <iostream>
#include "AndroidLogger.hpp"
#include "TimeHelper.hpp"
#include "UDPSender.h"
#include "UDPReceiver.h"
#include <cstring>
 
 static void fillBufferWithRandomData(std::vector<uint8_t>& data){
    const std::size_t size=data.size();
    for(std::size_t i=0;i<size;i++){
        data[i] = rand() % 255;
    }
}

// Create a buffer filled with random data of size sizeByes
std::vector<uint8_t> createRandomDataBuffer(const ssize_t sizeBytes){
  std::vector<uint8_t> buf(sizeBytes);
  fillBufferWithRandomData(buf);
  return buf;
}

struct PacketInfoData{
    uint32_t seqNr;
    std::chrono::steady_clock::time_point timestamp;
} __attribute__ ((packed));
//static_assert(sizeof(PacketInfoData)==4+8);

uint32_t currentSequenceNumber=0;

void writeSequenceNumberAndTimestamp(std::vector<uint8_t>& data){
    assert(data.size()>=sizeof(PacketInfoData));
    PacketInfoData* packetInfoData=(PacketInfoData*)data.data();
    packetInfoData->seqNr=currentSequenceNumber;
    packetInfoData->timestamp= std::chrono::steady_clock::now();
}

PacketInfoData getSequenceNumberAndTimestamp(const std::vector<uint8_t>& data){
    assert(data.size()>=sizeof(PacketInfoData));
    PacketInfoData packetInfoData;
    std::memcpy(&packetInfoData,data.data(),sizeof(PacketInfoData));
    return packetInfoData;
}

//AvgCalculator2 avgUDPProcessingTime{1024*1024};
AvgCalculator avgUDPProcessingTime;

static void validateReceivedData(const uint8_t* dataP,size_t data_length){
    auto data=std::vector<uint8_t>(dataP,dataP+data_length);
    const auto info=getSequenceNumberAndTimestamp(data);
    const auto latency=std::chrono::steady_clock::now()-info.timestamp;
	if(latency>std::chrono::milliseconds(1)){
		MLOGD<<"XGot data"<<data_length<<" "<<info.seqNr<<" "<<MyTimeHelper::R(latency)<<"\n";
	}
    //MLOGD<<"XGot data"<<data_length<<" "<<info.seqNr<<" "<<MyTimeHelper::R(latency)<<"\n";
    // do not use the first couple of packets, system needs to ramp up first
    //if(info.seqNr>10){
        avgUDPProcessingTime.add(latency);
    //}
}

static void generateDataPackets(std::function<void(std::vector<uint8_t>&)> cb,const int N_PACKETS,const int PACKET_SIZE,const int PACKETS_PER_SECOND){
    for(int i=0;i<N_PACKETS;i++){
        auto packet=createRandomDataBuffer(PACKET_SIZE);
        cb(packet);
    }
}

struct Options{
    const int PACKET_SIZE=1024;
    const int WANTED_PACKETS_PER_SECOND=1024;
    const int N_PACKETS=WANTED_PACKETS_PER_SECOND*5;
    const int INPUT_PORT=6001;
    const int OUTPUT_PORT=6001;
};


static void test_latency(const Options& o){
	const std::chrono::nanoseconds TIME_BETWEEN_PACKETS=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1))/o.WANTED_PACKETS_PER_SECOND;
    // start the receiver in its own thread
    UDPReceiver udpReceiver{nullptr,o.INPUT_PORT,"LTUdpRec",0,validateReceivedData,0};
    udpReceiver.startReceiving();
    // Wait a bit such that the OS can start the receiver before we start sending data
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    UDPSender udpSender{"127.0.0.1",o.OUTPUT_PORT};
	//UDPSender udpSender{"192.168.0.14",6002};
    currentSequenceNumber=0;
    avgUDPProcessingTime.reset();

    const std::chrono::steady_clock::time_point testBegin=std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point firstPacketTimePoint=std::chrono::steady_clock::now();
    std:size_t writtenBytes=0;
    std::size_t writtenPackets=0;
    for(int i=0;i<o.N_PACKETS;i++){
        auto buff=createRandomDataBuffer(o.PACKET_SIZE);
        writeSequenceNumberAndTimestamp(buff);
        udpSender.mySendTo(buff.data(),buff.size());
		//std::this_thread::sleep_for(std::chrono::microseconds(1));
		//validateReceivedData(buff.data(),buff.size());
        writtenBytes+=o.PACKET_SIZE;
        writtenPackets+=1;
        currentSequenceNumber++;
        // wait until as much time is elapsed such that we hit the target packets per seconds
        const auto timePointReadyToSendNextPacket=firstPacketTimePoint+i*TIME_BETWEEN_PACKETS;
        while(std::chrono::steady_clock::now()<timePointReadyToSendNextPacket){
            //ucomment for busy wait (no scheduler interruption)
			std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
    const auto testEnd=std::chrono::steady_clock::now();
    const double testTimeSeconds=(testEnd-testBegin).count()/1000.0f/1000.0f/1000.0f;
    const double actualPacketsPerSecond=(double)o.N_PACKETS/testTimeSeconds;
    const double actualMBytesPerSecond=(double)writtenBytes/testTimeSeconds/1024.0f/1024;

   // Wait for any packet that might be still in transit
   std::this_thread::sleep_for(std::chrono::seconds(1));
   udpReceiver.stopReceiving();

   MLOGD<<"Testing took:"<<testTimeSeconds<<"\n";
   MLOGD<<"WANTED_PACKETS_PER_SECOND "<<o.WANTED_PACKETS_PER_SECOND<<" Got "<<actualPacketsPerSecond<<
   " achieved bitrate: "<<actualMBytesPerSecond<<" MB/s"<<"\n";

   MLOGD<<"Avg UDP processing time "<<avgUDPProcessingTime.getAvgReadable()<<"\n";
   //MLOGD<<"All samples "<<avgUDPProcessingTime.getAllSamplesAsString();
}


int main(int argc, char *argv[])
{
	// For testing the localhost latency just use the same udp port for input and output
	// For testing wb latency TODO
	int opt;
    int ps=1024;
    int pps=2*1024;
    int wantedTime=5; // 5 seconds
	int input_port=6001;
	int output_port=6001;
    while ((opt = getopt(argc, argv, "s:p:t:i:o")) != -1) {
        switch (opt) {
        case 's':
            ps = atoi(optarg);
            break;
        case 'p':
            pps = atoi(optarg);
            break;
        case 't':
            wantedTime = atoi(optarg);
            break;
		case 'i':
			input_port=atoi(optarg);
			break;
		case 'o':
			output_port=atoi(optarg);
			break;
        default: /* '?' */
        show_usage:
            MLOGD<<"Usage: [-s=packet size in bytes] [-p=packets per second] [-t=time to run in seconds] [-i=input udp port] [-o=output udp port]\n";
            return 1;
        }
    }

    // For a packet size of 1024 bytes, 1024 packets per second equals 1 MB/s or 8 MBit/s
    // 8 MBit/s is a just enough for encoded 720p video
	const Options options{ps,pps,pps*wantedTime,input_port,output_port};
	test_latency(options);

    return 0;
}