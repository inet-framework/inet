//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/queue/AirtimeFairnessCompoundQueue.h"

#include "inet/common/Simsignals.h"

namespace inet {
namespace ieee80211 {

Define_Module(AirtimeFairnessCompoundQueue);

void AirtimeFairnessCompoundQueue::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    collection->removePacket(packet);
    // The frame was removed from its per-station sub-queue, but is still owned by that
    // sub-queue; take ownership so the base class's subsequent dropPacket() delete does not
    // trip the "deleting an object it doesn't own" warning.
    if (packet->getOwner() != this)
        take(packet);
    emit(packetRemovedSignal, packet);
}

} // namespace ieee80211
} // namespace inet
