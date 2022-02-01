//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/PacketSendToPush.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(PacketSendToPush);

void PacketSendToPush::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(outputGate);
}

void PacketSendToPush::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    handlePacketProcessed(packet);
    pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

} // namespace queueing
} // namespace inet

