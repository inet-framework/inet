#include <vector>
#include "inet/queueing/function/PacketSchedulerFunction.h"
#include "inet/common/packet/Packet.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

using namespace inet; 

static int testSchedule(const std::vector<queueing::IPassivePacketSource *>& providers)
{
    static int i = 0;
    return i++ % 2;
}

Register_Packet_Scheduler_Function(TestScheduler, testSchedule);
