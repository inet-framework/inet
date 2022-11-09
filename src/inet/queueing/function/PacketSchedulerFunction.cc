//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/function/PacketSchedulerFunction.h"

#include "inet/linklayer/common/UserPriorityTag_m.h"

namespace inet {
namespace queueing {

static int schedulePacketByData(const std::vector<IPassivePacketSource *>& sources)
{
    int selectedIndex = -1;
    int selectedData = -1;
    for (int i = 0; i < (int)sources.size(); i++) {
        auto source = sources[i];
        auto module = check_and_cast<cModule *>(source);
        auto packet = source->canPullPacket(module->gate("out"));
        if (packet != nullptr) {
            const auto& dataChunk = packet->peekDataAt<BytesChunk>(B(0), B(1));
            auto data = dataChunk->getBytes().at(0);
            if (data > selectedData) {
                selectedData = data;
                selectedIndex = i;
            }
        }
    }
    return selectedIndex;
}

Register_Packet_Scheduler_Function(PacketDataScheduler, schedulePacketByData);

static int schedulePacketByUserPriority(const std::vector<IPassivePacketSource *>& sources)
{
    int selectedIndex = -1;
    int selectedUserPriority = -1;
    for (int i = 0; i < (int)sources.size(); i++) {
        auto source = sources[i];
        auto module = check_and_cast<cModule *>(source);
        auto packet = source->canPullPacket(module->gate("out"));
        auto userPriorityReq = packet->getTag<UserPriorityReq>();
        if (userPriorityReq->getUserPriority() > selectedUserPriority) {
            selectedUserPriority = userPriorityReq->getUserPriority();
            selectedIndex = i;
        }
    }
    return selectedIndex;
}

Register_Packet_Scheduler_Function(PacketUserPriorityScheduler, schedulePacketByUserPriority);

static int schedulePacketByPacketLength(const std::vector<IPassivePacketSource *>& sources)
{
    int selectedIndex = -1;
    b selectedPacketLength = b(-1);
    for (int i = 0; i < (int)sources.size(); i++) {
        auto source = sources[i];
        auto module = check_and_cast<cModule *>(source);
        auto packet = source->canPullPacket(module->gate("out"));
        if (packet != nullptr) {
            auto packetLength = packet->getDataLength();
            if (packetLength > selectedPacketLength) {
                selectedPacketLength = packetLength;
                selectedIndex = i;
            }
        }
    }
    return selectedIndex;
}

Register_Packet_Scheduler_Function(PacketLengthScheduler, schedulePacketByPacketLength);

} // namespace queueing
} // namespace inet

