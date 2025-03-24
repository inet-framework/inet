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
}

void PacketProcessorBase::refreshDisplay() const
{
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
    if (consumer != nullptr) {
        animatePushPacket(packet, startGate, consumer.getReferencedGate());
        consumer.pushPacket(packet);
    }
    else
        send(packet, startGate);
}

void PacketProcessorBase::pushOrSendPacketStart(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer, bps datarate, int transmissionId)
{
    simtime_t duration = (packet->getDataLength() / datarate).get<s>();
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
    simtime_t remainingDuration = ((packet->getDataLength() - position) / datarate).get<s>();
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

void PacketProcessorBase::animate(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions, Action action) const
{
    packet->setIsUpdate(sendOptions.isUpdate);
    packet->setTransmissionId(sendOptions.transmissionId_);
    if (sendOptions.isUpdate && sendOptions.transmissionId_ == -1)
        throw cRuntimeError("No transmissionId specified in SendOptions for a transmission update");
    packet->setDuration(SIMTIME_ZERO);
    packet->setRemainingDuration(SIMTIME_ZERO);
    packet->setArrival(endGate->getOwnerModule()->getId(), endGate->getId(), simTime());
    packet->setSentFrom(startGate->getOwnerModule(), startGate->getId(), simTime());

#ifdef INET_WITH_SELFDOC
    if (SelfDoc::generateSelfdoc) {
        auto from = startGate->getOwnerModule();
        auto fromName = from->getComponentType()->getFullName();
        auto to = endGate->getOwnerModule();
        auto toName = to->getComponentType()->getFullName();
        auto ctrl = packet->getControlInfo();
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("module", fromName)
                    << ", " << SelfDoc::keyVal("action", action == PUSH ? "PUSH_OUT" : "PULLED_OUT")
                    << ", " << SelfDoc::val("details") << " : {"
                    << SelfDoc::keyVal("gate", SelfDoc::gateInfo(startGate))
                    << ", "<< SelfDoc::keyVal("msg", opp_typename(typeid(*packet)))
                    << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(packet->getKind(), startGate->getProperties(), "messageKinds", endGate->getProperties(), "messageKinds"))
                    << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                    << ", " << SelfDoc::tagsToJson("tags", packet)
                    << ", " << SelfDoc::keyVal("destModule", toName)
                    << " } }"
                    ;
            globalSelfDoc.insert(os.str());
        }
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("module", toName)
                    << ", " << SelfDoc::keyVal("action", action == PUSH ? "PUSHED_IN" : "PULL_IN")
                    << ", " << SelfDoc::val("details") << " : {"
                    << SelfDoc::keyVal("gate", SelfDoc::gateInfo(endGate))
                    << ", " << SelfDoc::keyVal("msg", opp_typename(typeid(*packet)))
                    << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(packet->getKind(), endGate->getProperties(), "messageKinds", startGate->getProperties(), "messageKinds"))
                    << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                    << ", " << SelfDoc::tagsToJson("tags", packet)
                    << ", " << SelfDoc::keyVal("srcModule", fromName)
                    << " } }"
                    ;
            globalSelfDoc.insert(os.str());
        }
    }
#endif // INET_WITH_SELFDOC

    auto envir = getEnvir();
    auto gate = startGate;
    if (gate->getNextGate() != nullptr) {
        envir->beginSend(packet, sendOptions);
        while (gate->getNextGate() != nullptr && gate != endGate) {
            ChannelResult result;
            result.duration = sendOptions.duration_;
            result.remainingDuration = sendOptions.remainingDuration;
            envir->messageSendHop(packet, gate, result);
            gate = gate->getNextGate();
        }
        envir->endSend(packet);
    }
    envir->pausePoint();
}

void PacketProcessorBase::animatePacket(Packet *packet, cGate *startGate, cGate *endGate, Action action) const
{
    SendOptions sendOptions;
    sendOptions.duration_ = 0;
    sendOptions.remainingDuration = 0;
    animate(packet, startGate, endGate, sendOptions, action);
}

void PacketProcessorBase::animatePacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId, Action action) const
{
    simtime_t duration = (packet->getDataLength() / datarate).get<s>();
    SendOptions sendOptions;
    sendOptions.duration_ = duration;
    sendOptions.remainingDuration = duration;
    sendOptions.transmissionId(transmissionId);
    animatePacketStart(packet, startGate, endGate, datarate, sendOptions, action);
}

void PacketProcessorBase::animatePacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions, Action action) const
{
    animate(packet, startGate, endGate, sendOptions, action);
}

void PacketProcessorBase::animatePacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId, Action action) const
{
    SendOptions sendOptions;
    sendOptions.updateTx(transmissionId, 0);
    animatePacketEnd(packet, startGate, endGate, sendOptions, action);
}

void PacketProcessorBase::animatePacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions, Action action) const
{
    animate(packet, startGate, endGate, sendOptions, action);
}

void PacketProcessorBase::animatePacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId, Action action) const
{
    SendOptions sendOptions;
    sendOptions.transmissionId(transmissionId);
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, sendOptions, action);
}

void PacketProcessorBase::animatePacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions, Action action) const
{
    animate(packet, startGate, endGate, sendOptions, action);
}

void PacketProcessorBase::animatePush(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions) const
{
    animate(packet, startGate, endGate, sendOptions, PUSH);
}

void PacketProcessorBase::animatePushPacket(Packet *packet, cGate *startGate, cGate *endGate) const
{
    animatePacket(packet, startGate, endGate, PUSH);
}

void PacketProcessorBase::animatePushPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId) const
{
    animatePacketStart(packet, startGate, endGate, datarate, transmissionId, PUSH);
}

void PacketProcessorBase::animatePushPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions) const
{
    animatePacketStart(packet, startGate, endGate, datarate, sendOptions, PUSH);
}

void PacketProcessorBase::animatePushPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId) const
{
    animatePacketEnd(packet, startGate, endGate, transmissionId, PUSH);
}

void PacketProcessorBase::animatePushPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions) const
{
    animatePacketEnd(packet, startGate, endGate, sendOptions, PUSH);
}

void PacketProcessorBase::animatePushPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId) const
{
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, transmissionId, PUSH);
}

void PacketProcessorBase::animatePushPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions) const
{
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, sendOptions, PUSH);
}

void PacketProcessorBase::animatePull(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions) const
{
    animate(packet, startGate, endGate, sendOptions, PULL);
}

void PacketProcessorBase::animatePullPacket(Packet *packet, cGate *startGate, cGate *endGate) const
{
    animatePacket(packet, startGate, endGate, PULL);
}

void PacketProcessorBase::animatePullPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId) const
{
    animatePacketStart(packet, startGate, endGate, datarate, transmissionId, PULL);
}

void PacketProcessorBase::animatePullPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions) const
{
    animatePacketStart(packet, startGate, endGate, datarate, sendOptions, PULL);
}

void PacketProcessorBase::animatePullPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId) const
{
    animatePacketEnd(packet, startGate, endGate, transmissionId, PULL);
}

void PacketProcessorBase::animatePullPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions) const
{
    animatePacketEnd(packet, startGate, endGate, sendOptions, PULL);
}

void PacketProcessorBase::animatePullPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId) const
{
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, transmissionId, PULL);
}

void PacketProcessorBase::animatePullPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions) const
{
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, sendOptions, PULL);
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

