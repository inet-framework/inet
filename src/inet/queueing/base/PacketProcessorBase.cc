//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketProcessorBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProgressTag_m.h"

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

void PacketProcessorBase::refreshDisplay() const
{
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
    if (gate->getPreviousGate() != nullptr)
        checkPacketOperationSupport(startGate, gate);
    if (gate->getNextGate() != nullptr)
        checkPacketOperationSupport(gate, endGate);
}

void PacketProcessorBase::checkPacketOperationSupport(cGate *startGate, cGate *endGate) const
{
    auto startElement = startGate == nullptr ? nullptr : check_and_cast<IPacketProcessor *>(startGate->getOwnerModule());
    auto endElement = endGate == nullptr ? nullptr : check_and_cast<IPacketProcessor *>(endGate->getOwnerModule());
    if (startElement != nullptr && endElement != nullptr) {
        bool startSending = startElement->supportsPacketSending(startGate);
        bool startPushing = startElement->supportsPacketPushing(startGate);
        bool startPulling = startElement->supportsPacketPulling(startGate);
        bool startPassing = startElement->supportsPacketPassing(startGate);
        bool startStreaming = startElement->supportsPacketStreaming(startGate);
        bool startOnlySending = startSending && !startPushing && !startPulling;
        bool endSending = endElement->supportsPacketSending(endGate);
        bool endPushing = endElement->supportsPacketPushing(endGate);
        bool endPulling = endElement->supportsPacketPulling(endGate);
        bool endPassing = endElement->supportsPacketPassing(endGate);
        bool endStreaming = endElement->supportsPacketStreaming(endGate);
        bool endOnlySending = endSending && !endPushing && !endPulling;
        bool bothPushing = startPushing && endPushing;
        bool bothPulling = startPulling && endPulling;
        bool bothPassing = startPassing && endPassing;
        bool bothStreaming = startStreaming && endStreaming;
        bool eitherPushing = startPushing || endPushing;
        bool eitherPulling = startPulling || endPulling;
        bool eitherPassing = startPassing || endPassing;
        bool eitherStreaming = startStreaming || endStreaming;
        if (startOnlySending || endOnlySending) {
            if (!startSending)
                throw cRuntimeError(startGate->getOwnerModule(), "doesn't support packet sending on gate %s", startGate->getFullPath().c_str());
            if (!endSending)
                throw cRuntimeError(endGate->getOwnerModule(), "doesn't support packet sending on gate %s", endGate->getFullPath().c_str());
        }
        else if (!bothPushing && !bothPulling) {
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
                throw cRuntimeError(endGate->getOwnerModule(), "neither supports packet pushing nor packet pulling on gates %s - %s", startGate->getFullPath().c_str(), endGate->getFullPath().c_str());
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
                throw cRuntimeError(endGate->getOwnerModule(), "neither supports packet passing nor packet streaming on gates %s - %s", startGate->getFullPath().c_str(), endGate->getFullPath().c_str());
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
        throw cRuntimeError("Cannot check packet operation support on gates %s - %s", startGate->getFullPath().c_str(), endGate->getFullPath().c_str());
}

void PacketProcessorBase::pushOrSendPacket(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer)
{
    if (consumer != nullptr)
        consumer.pushPacket(packet);
    else
        send(packet, startGate);
}

void PacketProcessorBase::pushOrSendPacketStart(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer, bps datarate, int transmissionId)
{
    simtime_t duration = s(packet->getDataLength() / datarate).get();
    SendOptions sendOptions;
    sendOptions.duration_ = duration;
    sendOptions.remainingDuration = duration;
    sendOptions.transmissionId(transmissionId);
    if (consumer != nullptr) {
        animatePushPacketStart(packet, startGate, consumer.getReferencedGate(), datarate, sendOptions);
        consumer.pushPacketStart(packet, datarate);
    }
    else {
        auto progressTag = packet->addTagIfAbsent<ProgressTag>();
        progressTag->setDatarate(datarate);
        progressTag->setPosition(b(0));
        send(packet, sendOptions, startGate);
    }
}

void PacketProcessorBase::pushOrSendPacketEnd(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer, int transmissionId)
{
    // NOTE: duration is unknown due to arbitrarily changing datarate
    SendOptions sendOptions;
    sendOptions.updateTx(transmissionId, 0);
    if (consumer != nullptr) {
        animatePushPacketEnd(packet, startGate, consumer.getReferencedGate(), sendOptions);
        consumer.pushPacketEnd(packet);
    }
    else {
        auto progressTag = packet->addTagIfAbsent<ProgressTag>();
        progressTag->setDatarate(bps(NaN));
        progressTag->setPosition(packet->getDataLength());
        send(packet, sendOptions, startGate);
    }
}

void PacketProcessorBase::pushOrSendPacketProgress(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer, bps datarate, b position, b extraProcessableLength, int transmissionId)
{
    // NOTE: duration is unknown due to arbitrarily changing datarate
    simtime_t remainingDuration = s((packet->getDataLength() - position) / datarate).get();
    SendOptions sendOptions;
    sendOptions.updateTx(transmissionId, remainingDuration);
    if (consumer != nullptr) {
        if (position == b(0)) {
            animatePushPacketStart(packet, startGate, consumer.getReferencedGate(), datarate, sendOptions);
            consumer.pushPacketStart(packet, datarate);
        }
        else if (position == packet->getDataLength()) {
            animatePushPacketEnd(packet, startGate, consumer.getReferencedGate(), sendOptions);
            consumer.pushPacketEnd(packet);
        }
        else {
            animatePushPacketProgress(packet, startGate, consumer.getReferencedGate(), datarate, position, extraProcessableLength, sendOptions);
            consumer.pushPacketProgress(packet, datarate, position, extraProcessableLength);
        }
    }
    else {
        auto progressTag = packet->addTagIfAbsent<ProgressTag>();
        progressTag->setDatarate(datarate);
        progressTag->setPosition(position);
        progressTag->setExtraProcessableLength(extraProcessableLength);
        send(packet, sendOptions, startGate);
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
    if (getEnvir()->isGUI() && displayStringTextFormat != nullptr) {
        auto text = StringFormat::formatString(displayStringTextFormat, this);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
}

std::string PacketProcessorBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'p':
            return std::to_string(numProcessedPackets);
        case 'l':
            return processedTotalLength.str();
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

} // namespace queueing
} // namespace inet

