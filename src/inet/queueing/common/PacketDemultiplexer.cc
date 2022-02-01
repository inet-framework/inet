//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/PacketDemultiplexer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

Define_Module(PacketDemultiplexer);

void PacketDemultiplexer::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        for (int i = 0; i < gateSize("out"); i++) {
            auto outputGate = gate("out", i);
            auto output = getConnectedModule<IActivePacketSink>(outputGate);
            outputGates.push_back(outputGate);
            collectors.push_back(output);
        }
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        for (auto outputGate : outputGates)
            checkPacketOperationSupport(outputGate);
    }
}

Packet *PacketDemultiplexer::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = provider->pullPacket(inputGate->getPathStartGate());
    take(packet);
    EV_INFO << "Forwarding packet" << EV_FIELD(packet) << EV_ENDL;
    animatePullPacket(packet, gate);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    return packet;
}

void PacketDemultiplexer::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    for (int i = 0; i < (int)outputGates.size(); i++)
        // NOTE: notifying a listener may prevent others from pulling
        if (collectors[i] != nullptr && provider->canPullSomePacket(inputGate->getPathStartGate()))
            collectors[i]->handleCanPullPacketChanged(outputGates[i]->getPathEndGate());
}

void PacketDemultiplexer::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    throw cRuntimeError("Invalid operation");
}

} // namespace queueing
} // namespace inet

