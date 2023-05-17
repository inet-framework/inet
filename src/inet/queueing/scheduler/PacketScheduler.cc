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
    // TODO: should we change the interface method instead?
    std::vector<IPassivePacketSource *> rawProviders;
    for (auto provider : providers)
        rawProviders.push_back(provider.get());
    int index = packetSchedulerFunction->schedulePacket(rawProviders);
    return index == -1 ? index : getInputGateIndex(index);
}

} // namespace queueing
} // namespace inet

