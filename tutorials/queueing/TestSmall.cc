#include "inet/queueing/function/PacketFilterFunction.h"
#include "inet/common/packet/Packet.h"

using namespace inet; 

static bool testSmall(Packet *packet)
{
    return packet->getTotalLength() == B(1);
}

Register_Packet_Filter_Function(TestSmall, testSmall);

static bool testLarge(Packet *packet)
{
    return packet->getTotalLength() == B(2);
}

Register_Packet_Filter_Function(TestLarge, testLarge);
