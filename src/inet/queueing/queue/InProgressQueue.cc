//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/queue/InProgressQueue.h"

namespace inet {
namespace queueing {

Define_Module(InProgressQueue);

bool InProgressQueue::canPushPacket(Packet *packet, cGate *gate) const
{
    return queue.isEmpty() || packetComparatorFunction->comparePackets(packet, getPacket(0)) > 0;
}

} // namespace queueing
} // namespace inet

