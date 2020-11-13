//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/queueing/base/PacketProcessorBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/common/ProgressTag_m.h"

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

void PacketProcessorBase::pushOrSendPacket(Packet *packet, cGate *gate, IPassivePacketSink *consumer)
{
    if (consumer != nullptr) {
        animateSendPacket(packet, gate);
        consumer->pushPacket(packet, gate->getPathEndGate());
    }
    else
        send(packet, gate);
}

void PacketProcessorBase::pushOrSendPacketStart(Packet *packet, cGate *gate, IPassivePacketSink *consumer, bps datarate, int transmissionId)
{
    simtime_t duration = s(packet->getTotalLength() / datarate).get();
    SendOptions sendOptions;
    sendOptions.duration(duration);
    sendOptions.updateTx(transmissionId, duration);
    if (consumer != nullptr) {
        animateSendPacketStart(packet, gate, datarate, sendOptions);
        consumer->pushPacketStart(packet, gate->getPathEndGate(), datarate);
    }
    else {
        auto progressTag = packet->addTagIfAbsent<ProgressTag>();
        progressTag->setDatarate(datarate);
        progressTag->setPosition(b(0));
        send(packet, sendOptions, gate);
    }
}

void PacketProcessorBase::pushOrSendPacketEnd(Packet *packet, cGate *gate, IPassivePacketSink *consumer, int transmissionId)
{
    // NOTE: duration is unknown due to arbitrarily changing datarate
    SendOptions sendOptions;
    sendOptions.updateTx(transmissionId, 0);
    if (consumer != nullptr) {
        animateSendPacketEnd(packet, gate, sendOptions);
        consumer->pushPacketEnd(packet, gate->getPathEndGate());
    }
    else {
        auto progressTag = packet->addTagIfAbsent<ProgressTag>();
        progressTag->setDatarate(bps(NaN));
        progressTag->setPosition(packet->getTotalLength());
        send(packet, sendOptions, gate);
    }
}

void PacketProcessorBase::pushOrSendPacketProgress(Packet *packet, cGate *gate, IPassivePacketSink *consumer, bps datarate, b position, b extraProcessableLength, int transmissionId)
{
    // NOTE: duration is unknown due to arbitrarily changing datarate
    simtime_t remainingDuration = s((packet->getTotalLength() - position) / datarate).get();
    SendOptions sendOptions;
    sendOptions.updateTx(transmissionId, remainingDuration);
    if (consumer != nullptr) {
        if (position == b(0)) {
            animateSendPacketStart(packet, gate, datarate, sendOptions);
            consumer->pushPacketStart(packet, gate->getPathEndGate(), datarate);
        }
        else if (position == packet->getTotalLength()) {
            animateSendPacketEnd(packet, gate, sendOptions);
            consumer->pushPacketEnd(packet, gate->getPathEndGate());
        }
        else {
            animateSendPacketProgress(packet, gate, datarate, position, extraProcessableLength, sendOptions);
            consumer->pushPacketProgress(packet, gate->getPathEndGate(), datarate, position, extraProcessableLength);
        }
    }
    else {
        auto progressTag = packet->addTagIfAbsent<ProgressTag>();
        progressTag->setDatarate(datarate);
        progressTag->setPosition(position);
        progressTag->setExtraProcessableLength(extraProcessableLength);
        send(packet, sendOptions, gate);
    }
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

void PacketProcessorBase::animateSend(Packet *packet, cGate *gate, const SendOptions& sendOptions) const
{
    auto endGate = gate->getPathEndGate();
    packet->setTransmissionId(sendOptions.transmissionId_);
    packet->setArrival(endGate->getOwnerModule()->getId(), endGate->getId(), simTime());
    packet->setSentFrom(gate->getOwnerModule(), gate->getId(), simTime());
    auto envir = getEnvir();
    if (envir->isGUI() && gate->getNextGate() != nullptr) {
        envir->beginSend(packet, sendOptions);
        while (gate->getNextGate() != nullptr) {
            ChannelResult result;
            result.duration = sendOptions.duration_;
            result.remainingDuration = sendOptions.remainingDuration;
            envir->messageSendHop(packet, gate, result);
            gate = gate->getNextGate();
        }
        envir->endSend(packet);
    }
}

void PacketProcessorBase::animateSendPacket(Packet *packet, cGate *gate) const
{
    animateSend(packet, gate, SendOptions());
}

void PacketProcessorBase::animateSendPacketStart(Packet *packet, cGate *gate, bps datarate, const SendOptions& sendOptions) const
{
    animateSend(packet, gate, sendOptions);
}

void PacketProcessorBase::animateSendPacketEnd(Packet *packet, cGate *gate, const SendOptions& sendOptions) const
{
    animateSend(packet, gate, sendOptions);
}

void PacketProcessorBase::animateSendPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions) const
{
    animateSend(packet, gate, sendOptions);
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

