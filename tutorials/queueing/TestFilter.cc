#include "inet/queueing/function/PacketFilterFunction.h"
#include "inet/common/packet/Packet.h"

using namespace inet; 

static bool testFilter(Packet *packet)
{
    auto name = packet->getName();
    if (name == nullptr)
        return false;
    else {
        auto length = strlen(name);
        return length != 0 && *(name + length - 1) % 2 == 0;
    }
}

Register_Packet_Filter_Function(TestFilter, testFilter);
