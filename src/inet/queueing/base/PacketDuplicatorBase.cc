//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketDuplicatorBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketDuplicatorBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketDuplicatorBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    int numDuplicates = getNumPacketDuplicates(packet);
    for (int i = 0; i < numDuplicates; i++) {
        EV_INFO << "Forwarding duplicate packet" << EV_FIELD(packet) << EV_ENDL;
        auto duplicate = packet->dup();
        pushOrSendPacket(duplicate, outputGate, consumer);
    }
    EV_INFO << "Forwarding original packet" << EV_FIELD(packet) << EV_ENDL;
    pushOrSendPacket(packet, outputGate, consumer);
}

void PacketDuplicatorBase::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

} // namespace queueing
} // namespace inet

