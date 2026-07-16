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

void AirtimeFairnessCompoundQueue::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    cNamedObject packetPushStartedDetails("atomicOperationStarted");
    emit(packetPushStartedSignal, packet, &packetPushStartedDetails);
    animatePushPacket(packet, inputGate, consumer.getReferencedGate());
    EV_INFO << "Pushing packet" << EV_FIELD(packet) << EV_ENDL;
    consumer.pushPacket(packet);
    // Shared-capacity overflow: drop from the longest per-station backlog. Unlike the base
    // class, remove the victim directly from its sub-queue collection instead of via
    // removePacket(): removePacket() would emit packetRemoved, making the drop count as both
    // a removal and a drop (subtracted twice from the queue-length statistic). Take ownership
    // of the victim (it is still owned by its sub-queue) before dropPacket() deletes it, so
    // the delete does not warn about deleting an object owned by another module.
    while (packetDropperFunction != nullptr && isOverloaded()) {
        auto droppedPacket = packetDropperFunction->selectPacket(this);
        EV_INFO << "Dropping packet from the longest sub-queue" << EV_FIELD(droppedPacket) << EV_ENDL;
        collection->removePacket(droppedPacket);
        if (droppedPacket->getOwner() != this)
            take(droppedPacket);
        dropPacket(droppedPacket, QUEUE_OVERFLOW);
    }
    ASSERT(!isOverloaded());
    cNamedObject packetPushEndedDetails("atomicOperationEnded");
    emit(packetPushEndedSignal, nullptr, &packetPushEndedDetails);
}

} // namespace ieee80211
} // namespace inet
