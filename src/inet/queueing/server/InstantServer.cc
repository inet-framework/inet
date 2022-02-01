//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/server/InstantServer.h"

#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

Define_Module(InstantServer);

bool InstantServer::canProcessPacket()
{
    auto inputGatePathStartGate = inputGate->getPathStartGate();
    auto outputGatePathEndGate = outputGate->getPathEndGate();
    if (provider->canPullSomePacket(inputGatePathStartGate) && consumer->canPushSomePacket(outputGatePathEndGate))
        return true;
    else {
        auto packet = provider->canPullPacket(inputGatePathStartGate);
        return packet != nullptr && consumer->canPushPacket(packet, outputGatePathEndGate);
    }
}

void InstantServer::processPacket()
{
    auto packet = provider->pullPacket(inputGate->getPathStartGate());
    take(packet);
    emit(packetPulledSignal, packet);
    std::string packetName = packet->getName();
    auto packetLength = packet->getDataLength();
    EV_INFO << "Processing packet started" << EV_FIELD(packet) << EV_ENDL;
    emit(packetPushedSignal, packet);
    pushOrSendPacket(packet, outputGate, consumer);
    processedTotalLength += packetLength;
    numProcessedPackets++;
    EV_INFO << "Processing packet ended" << EV_ENDL;
}

void InstantServer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    processPackets();
}

void InstantServer::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    processPackets();
}

void InstantServer::processPackets()
{
    if (!isProcessing) {
        isProcessing = true;
        while (canProcessPacket())
            processPacket();
        isProcessing = false;
        updateDisplayString();
    }
}

} // namespace queueing
} // namespace inet

