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
            outputGates.push_back(outputGate);
            ActivePacketSinkRef collector;
            collector.reference(outputGate, true);
            collectors.push_back(collector);
        }
        provider.reference(inputGate, false);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        for (auto outputGate : outputGates)
            checkPacketOperationSupport(outputGate);
    }
}

Packet *PacketDemultiplexer::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = provider.pullPacket();
    take(packet);
    EV_INFO << "Forwarding packet" << EV_FIELD(packet) << EV_ENDL;
    animatePullPacket(packet, outputGates[gate->getIndex()], findConnectedGate<IActivePacketSink>(gate));
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    return packet;
}

void PacketDemultiplexer::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    for (size_t i = 0; i < outputGates.size(); i++)
        // NOTE: notifying a listener may prevent others from pulling
        if (collectors[i] != nullptr && provider.canPullSomePacket())
            collectors[i].handleCanPullPacketChanged();
}

void PacketDemultiplexer::handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    throw cRuntimeError("Invalid operation");
}

} // namespace queueing
} // namespace inet

