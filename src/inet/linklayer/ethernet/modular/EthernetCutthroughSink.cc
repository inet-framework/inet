//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetCutthroughSink.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(EthernetCutthroughSink);

void EthernetCutthroughSink::initialize(int stage)
{
    PacketStreamer::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        cutthroughInputGate = gate("cutthroughIn");
        cutthroughProducer = findConnectedModule<IActivePacketSource>(cutthroughInputGate);
    }
}

bool EthernetCutthroughSink::canPushPacket(Packet *packet, cGate *gate) const
{
    if (gate == cutthroughInputGate)
        return !isStreaming() && !provider->canPullSomePacket(inputGate->getPathStartGate()) && consumer->canPushPacket(packet, outputGate->getPathEndGate());
    else
        return PacketStreamer::canPushPacket(packet, gate);
}

void EthernetCutthroughSink::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    if (gate == cutthroughInputGate) {
        Enter_Method("pushPacketStart");
        take(packet);
        delete streamedPacket;
        streamedPacket = packet;
        cutthrough = true;
        EV_INFO << "Starting streaming packet " << streamedPacket->getName() << "." << std::endl;
        pushOrSendPacketStart(streamedPacket, outputGate, consumer, datarate, streamedPacket->getTransmissionId());
        streamedPacket = nullptr;
        updateDisplayString();
    }
    else {
        cutthrough = false;
        PacketStreamer::pushPacketStart(packet, gate, datarate);
    }
}

void EthernetCutthroughSink::pushPacketEnd(Packet *packet, cGate *gate)
{
    if (gate == cutthroughInputGate) {
        Enter_Method("pushPacketEnd");
        take(packet);
        delete streamedPacket;
        streamedPacket = packet;
        EV_INFO << "Ending streaming packet " << streamedPacket->getName() << "." << std::endl;
        pushOrSendPacketEnd(streamedPacket, outputGate, consumer, streamedPacket->getTransmissionId());
        streamedPacket = nullptr;
        updateDisplayString();
    }
    else
        PacketStreamer::pushPacketEnd(packet, gate);
}

void EthernetCutthroughSink::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (!cutthrough && producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
    else if (cutthrough && cutthroughProducer != nullptr)
        cutthroughProducer->handlePushPacketProcessed(packet, cutthroughInputGate->getPathStartGate(), successful);
}

} // namespace inet

