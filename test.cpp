// Program: MyArgs
#include <iostream>

#include "AndroidLogger.hpp"
#include "TimeHelper.hpp"
#include "UDPSender.h"
#include "UDPReceiver.h"
 
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
static_assert(sizeof(PacketInfoData)==4+8);

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
    memcpy(&packetInfoData,data.data(),sizeof(PacketInfoData));
    return packetInfoData;
}

AvgCalculator avgUDPProcessingTime;

static void validateReceivedData(const uint8_t* dataP,size_t data_length){
    auto data=std::vector<uint8_t>(dataP,dataP+data_length);
    const auto info=getSequenceNumberAndTimestamp(data);
    const auto latency=std::chrono::steady_clock::now()-info.timestamp;
    MLOGD<<"XGot data"<<data_length<<" "<<info.seqNr<<" "<<MyTimeHelper::R(latency);
    // do not use the first couple of packets, system needs to ramp up first
    if(info.seqNr>10){
        avgUDPProcessingTime.add(latency);
    }
}

static void generateDataPackets(std::function<void(std::vector<uint8_t>&)> cb,const int N_PACKETS,const int PACKET_SIZE,const int PACKETS_PER_SECOND){
    for(int i=0;i<N_PACKETS;i++){
        auto packet=createRandomDataBuffer(PACKET_SIZE);
        cb(packet);
    }
}

int main(int argc, char *argv[])
{
    std::cout << "There are " << argc << " arguments:\n";
 
    // Loop through each argument and print its number and value
    for (int count{ 0 }; count < argc; ++count)
    {
        std::cout << count << ' ' << argv[count] << '\n';
    }
 
    return 0;
}