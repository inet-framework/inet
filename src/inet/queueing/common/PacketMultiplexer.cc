//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/PacketMultiplexer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(PacketMultiplexer);

void PacketMultiplexer::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        forwardServiceRegistration = par("forwardServiceRegistration");
        forwardProtocolRegistration = par("forwardProtocolRegistration");
        for (int i = 0; i < gateSize("in"); i++) {
            auto inputGate = gate("in", i);
            auto input = findConnectedModule<IActivePacketSource>(inputGate);
            inputGates.push_back(inputGate);
            producers.push_back(input);
        }
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (auto& inputGate : inputGates)
            checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketMultiplexer::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketMultiplexer::mapRegistrationForwardingGates(cGate *gate, std::function<void(cGate *)> f)
{
    if (gate == outputGate) {
        for (auto inputGate : inputGates)
            f(inputGate);
    }
    else if (std::find(inputGates.begin(), inputGates.end(), gate) != inputGates.end())
        f(outputGate);
    else
        throw cRuntimeError("Unknown gate");
}

void PacketMultiplexer::checkPacketStreaming(Packet *packet)
{
    if (inProgressStreamId != -1 && (packet == nullptr || packet->getTreeId() != inProgressStreamId))
        throw cRuntimeError("Another packet streaming operation is already in progress");
}

void PacketMultiplexer::startPacketStreaming(Packet *packet)
{
    inProgressStreamId = packet->getTreeId();
}

void PacketMultiplexer::endPacketStreaming(Packet *packet)
{
    emit(packetPushedSignal, packet);
    handlePacketProcessed(packet);
    inProgressStreamId = -1;
}

void PacketMultiplexer::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    EV_INFO << "Forwarding packet" << EV_FIELD(packet) << EV_ENDL;
    handlePacketProcessed(packet);
    pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

void PacketMultiplexer::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    EV_INFO << "Forwarding packet" << EV_FIELD(packet) << EV_ENDL;
    checkPacketStreaming(packet);
    startPacketStreaming(packet);
    pushOrSendPacketStart(packet, outputGate, consumer, datarate, packet->getTransmissionId());
    updateDisplayString();
}

void PacketMultiplexer::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    EV_INFO << "Forwarding packet" << EV_FIELD(packet) << EV_ENDL;
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    endPacketStreaming(packet);
    pushOrSendPacketEnd(packet, outputGate, consumer, packet->getTransmissionId());
    updateDisplayString();
}

void PacketMultiplexer::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    EV_INFO << "Forwarding packet" << EV_FIELD(packet) << EV_ENDL;
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    if (packet->getTotalLength() == position + extraProcessableLength)
        endPacketStreaming(packet);
    pushOrSendPacketProgress(packet, outputGate, consumer, datarate, position, extraProcessableLength, packet->getTransmissionId());
    updateDisplayString();
}

void PacketMultiplexer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    for (int i = 0; i < (int)inputGates.size(); i++)
        // NOTE: notifying a listener may prevent others from pushing
        if (producers[i] != nullptr && consumer->canPushSomePacket(outputGate))
            producers[i]->handleCanPushPacketChanged(inputGates[i]->getPathStartGate());
}

void PacketMultiplexer::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
}

} // namespace queueing
} // namespace inet

