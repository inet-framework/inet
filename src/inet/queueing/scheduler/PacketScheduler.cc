//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/scheduler/PacketScheduler.h"

namespace inet {
namespace queueing {

Define_Module(PacketScheduler);

void PacketScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        packetSchedulerFunction = createSchedulerFunction(par("schedulerClass"));
}

IPacketSchedulerFunction *PacketScheduler::createSchedulerFunction(const char *schedulerClass) const
{
    return check_and_cast<IPacketSchedulerFunction *>(createOne(schedulerClass));
}

int PacketScheduler::schedulePacket()
{
    int index = packetSchedulerFunction->schedulePacket(providers);
    return index == -1 ? index : getInputGateIndex(index);
}

} // namespace queueing
} // namespace inet

