//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/base/PacketProcessorBase.h"

namespace inet {
namespace queueing {

void PacketProcessorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        numProcessedPackets = 0;
        processedTotalLength = b(0);
        WATCH(numProcessedPackets);
        WATCH(processedTotalLength);
    }
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

void PacketProcessorBase::handlePacketProcessed(Packet *packet)
{
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
}

void PacketProcessorBase::checkPacketOperationSupport(cGate *gate) const
{
    auto startGate = findConnectedGate<IPacketProcessor>(gate, -1);
    auto endGate = findConnectedGate<IPacketProcessor>(gate, 1);
    auto startElement = startGate == nullptr ? nullptr : check_and_cast<IPacketProcessor *>(startGate->getOwnerModule());
    auto endElement = endGate == nullptr ? nullptr : check_and_cast<IPacketProcessor *>(endGate->getOwnerModule());
    if (startElement != nullptr && endElement != nullptr) {
        bool startPushing = startElement->supportsPacketPushing(startGate);
        bool startPulling = startElement->supportsPacketPulling(startGate);
        bool startPassing = startElement->supportsPacketPassing(startGate);
        bool startStreaming = startElement->supportsPacketStreaming(startGate);
        bool endPushing = endElement->supportsPacketPushing(endGate);
        bool endPulling = endElement->supportsPacketPulling(endGate);
        bool endPassing = endElement->supportsPacketPassing(endGate);
        bool endStreaming = endElement->supportsPacketStreaming(endGate);
        bool bothPushing = startPushing && endPushing;
        bool bothPulling = startPulling && endPulling;
        bool bothPassing = startPassing && endPassing;
        bool bothStreaming = startStreaming && endStreaming;
        bool eitherPushing = startPushing || endPushing;
        bool eitherPulling = startPulling || endPulling;
        bool eitherPassing = startPassing || endPassing;
        bool eitherStreaming = startStreaming || endStreaming;
        if (!bothPushing && !bothPulling) {
            if (eitherPushing) {
                if (startPushing)
                    throw cRuntimeError(endGate->getOwnerModule(), "doesn't support packet pushing on gate %s", endGate->getFullPath().c_str());
                if (endPushing)
                    throw cRuntimeError(startGate->getOwnerModule(), "doesn't support packet pushing on gate %s", startGate->getFullPath().c_str());
            }
            else if (eitherPulling) {
                if (startPulling)
                    throw cRuntimeError(endGate->getOwnerModule(), "doesn't support packet pulling on gate %s", endGate->getFullPath().c_str());
                if (endPulling)
                    throw cRuntimeError(startGate->getOwnerModule(), "doesn't support packet pulling on gate %s", startGate->getFullPath().c_str());
            }
            else
                throw cRuntimeError(endGate->getOwnerModule(), "neither supports packet pushing nor packet pulling on gate %s", gate->getFullPath().c_str());
        }
        if (!bothPassing && !bothStreaming) {
            if (eitherPassing) {
                if (startPassing)
                    throw cRuntimeError(endGate->getOwnerModule(), "doesn't support packet passing on gate %s", endGate->getFullPath().c_str());
                if (endPassing)
                    throw cRuntimeError(startGate->getOwnerModule(), "doesn't support packet passing on gate %s", startGate->getFullPath().c_str());
            }
            else if (eitherStreaming) {
                if (startStreaming)
                    throw cRuntimeError(endGate->getOwnerModule(), "doesn't support packet streaming on gate %s", endGate->getFullPath().c_str());
                if (endStreaming)
                    throw cRuntimeError(startGate->getOwnerModule(), "doesn't support packet streaming on gate %s", startGate->getFullPath().c_str());
            }
            else
                throw cRuntimeError(endGate->getOwnerModule(), "neither supports packet passing nor packet streaming on gate %s", gate->getFullPath().c_str());
        }
    }
    else if (startElement != nullptr && endElement == nullptr) {
        if (!startElement->supportsPacketSending(startGate))
            throw cRuntimeError(startGate->getOwnerModule(), "doesn't support packet sending on gate %s", startGate->getFullPath().c_str());
    }
    else if (startElement == nullptr && endElement != nullptr) {
        if (!endElement->supportsPacketSending(endGate))
            throw cRuntimeError(endGate->getOwnerModule(), "doesn't support packet sending on gate %s", endGate->getFullPath().c_str());
    }
    else
        throw cRuntimeError("Cannot check packet operation support on gate %s", gate->getFullPath().c_str());
}

void PacketProcessorBase::pushOrSendPacket(Packet *packet, cGate *gate)
{
    auto consumer = findConnectedModule<IPassivePacketSink>(gate);
    pushOrSendPacket(packet, gate, consumer);
}

void PacketProcessorBase::pushOrSendPacket(Packet *packet, cGate *gate, IPassivePacketSink *consumer)
{
    if (consumer != nullptr) {
        animateSend(packet, gate);
        consumer->pushPacket(packet, gate->getPathEndGate());
    }
    else
        send(packet, gate);
}

void PacketProcessorBase::pushOrSendPacketStart(Packet *packet, cGate *gate, IPassivePacketSink *consumer, bps datarate)
{
    if (consumer != nullptr) {
        animateSend(packet, gate);
        consumer->pushPacketStart(packet, gate->getPathEndGate(), datarate);
    }
    else
        sendPacketStart(packet, gate, 0);
}

void PacketProcessorBase::pushOrSendPacketEnd(Packet *packet, cGate *gate, IPassivePacketSink *consumer, bps datarate)
{
    if (consumer != nullptr) {
        animateSend(packet, gate);
        consumer->pushPacketEnd(packet, gate->getPathEndGate(), datarate);
    }
    else
        sendPacketEnd(packet, gate, 0);
}

void PacketProcessorBase::pushOrSendPacketProgress(Packet *packet, cGate *gate, IPassivePacketSink *consumer, bps datarate, b position, b extraProcessableLength)
{
    if (consumer != nullptr) {
        animateSend(packet, gate);
        consumer->pushPacketProgress(packet, gate->getPathEndGate(), datarate, position, extraProcessableLength);
    }
    else
        sendPacketProgress(packet, gate, 0, b(position).get(), 0, b(extraProcessableLength).get(), 0);
}

void PacketProcessorBase::dropPacket(Packet *packet, PacketDropReason reason, int limit)
{
    PacketDropDetails details;
    details.setReason(reason);
    details.setLimit(limit);
    emit(packetDroppedSignal, packet, &details);
    delete packet;
}

void PacketProcessorBase::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(displayStringTextFormat, this);
        getDisplayString().setTagArg("t", 0, text);
    }
}

void PacketProcessorBase::animateSend(Packet *packet, cGate *gate)
{
    auto envir = getEnvir();
    if (envir->isGUI()) {
        auto endGate = gate->getPathEndGate();
        packet->setSentFrom(gate->getOwnerModule(), gate->getId(), simTime());
        packet->setArrival(endGate->getOwnerModule()->getId(), endGate->getId(), simTime());
        if (gate->getNextGate() != nullptr) {
            envir->beginSend(packet);
            while (gate->getNextGate() != nullptr) {
                envir->messageSendHop(packet, gate);
                gate = gate->getNextGate();
            }
            envir->endSend(packet);
        }
    }
}

const char *PacketProcessorBase::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'p':
            result = std::to_string(numProcessedPackets);
            break;
        case 'l':
            result = processedTotalLength.str();
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

