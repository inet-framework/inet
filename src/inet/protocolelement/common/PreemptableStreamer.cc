//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/common/PreemptableStreamer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(PreemptableStreamer);

PreemptableStreamer::~PreemptableStreamer()
{
    delete streamedPacket;
    delete remainingPacket;
    cancelAndDeleteClockEvent(endStreamingTimer);
}

void PreemptableStreamer::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        datarate = bps(par("datarate"));
        minPacketLength = b(par("minPacketLength"));
        roundingLength = b(par("roundingLength"));
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        collector = findConnectedModule<IActivePacketSink>(outputGate);
        endStreamingTimer = new ClockEvent("EndStreamingTimer");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PreemptableStreamer::handleMessage(cMessage *message)
{
    if (message == endStreamingTimer)
        endStreaming();
    else {
        auto packet = check_and_cast<Packet *>(message);
        pushPacket(packet, packet->getArrivalGate());
    }
}

void PreemptableStreamer::endStreaming()
{
    auto packetLength = streamedPacket->getDataLength();
    EV_INFO << "Ending streaming packet" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer, streamedPacket->getId());
    streamDatarate = bps(NaN);
    streamedPacket = nullptr;
    numProcessedPackets++;
    processedTotalLength += packetLength;
    updateDisplayString();
}

bool PreemptableStreamer::canPushSomePacket(cGate *gate) const
{
    return !isStreaming() && consumer->canPushSomePacket(outputGate->getPathEndGate());
}

bool PreemptableStreamer::canPushPacket(Packet *packet, cGate *gate) const
{
    return !isStreaming() && consumer->canPushPacket(packet, outputGate->getPathEndGate());
}

void PreemptableStreamer::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    ASSERT(!isStreaming());
    take(packet);
    streamDatarate = datarate;
    streamedPacket = packet;
    EV_INFO << "Starting streaming packet" << EV_FIELD(packet) << EV_ENDL;
    pushOrSendPacketStart(streamedPacket->dup(), outputGate, consumer, datarate, streamedPacket->getId());
    if (std::isnan(streamDatarate.get()))
        endStreaming();
    else
        scheduleClockEventAfter(s(streamedPacket->getDataLength() / streamDatarate).get(), endStreamingTimer);
}

void PreemptableStreamer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void PreemptableStreamer::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

bool PreemptableStreamer::canPullSomePacket(cGate *gate) const
{
    return !isStreaming() && (remainingPacket != nullptr || provider->canPullSomePacket(inputGate->getPathStartGate()));
}

Packet *PreemptableStreamer::canPullPacket(cGate *gate) const
{
    return isStreaming() ? nullptr : remainingPacket != nullptr ? remainingPacket : provider->canPullPacket(inputGate->getPathStartGate());
}

Packet *PreemptableStreamer::pullPacketStart(cGate *gate, bps datarate)
{
    Enter_Method("pullPacketStart");
    streamDatarate = datarate;
    if (remainingPacket == nullptr) {
        streamedPacket = provider->pullPacket(inputGate->getPathStartGate());
        take(streamedPacket);
    }
    else {
        streamedPacket = remainingPacket;
        remainingPacket = nullptr;
    }
    auto fragmentTag = streamedPacket->findTagForUpdate<FragmentTag>();
    if (fragmentTag == nullptr) {
        fragmentTag = streamedPacket->addTag<FragmentTag>();
        fragmentTag->setFirstFragment(true);
        fragmentTag->setLastFragment(true);
        fragmentTag->setFragmentNumber(0);
        fragmentTag->setNumFragments(-1);
    }
    streamStart = simTime();
    auto packet = streamedPacket->dup();
    EV_INFO << "Starting streaming packet" << EV_FIELD(packet) << EV_ENDL;
    animatePullPacketStart(packet, outputGate, streamDatarate, streamedPacket->getId());
    updateDisplayString();
    return packet;
}

Packet *PreemptableStreamer::pullPacketEnd(cGate *gate)
{
    Enter_Method("pullPacketEnd");
    EV_INFO << "Ending streaming packet" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    auto packet = streamedPacket;
    b pulledLength = streamDatarate * s((simTime() - streamStart).dbl());
    b preemptedLength = roundingLength * ((pulledLength + roundingLength - b(1)) / roundingLength);
    if (preemptedLength < minPacketLength)
        preemptedLength = minPacketLength;
    if (preemptedLength + minPacketLength <= packet->getDataLength()) {
        // already pulled part
        const auto& fragmentTag = packet->getTagForUpdate<FragmentTag>();
        fragmentTag->setLastFragment(false);
        auto fragmentNumber = fragmentTag->getFragmentNumber();
        std::string basePacketName = packet->getName();
        if (fragmentNumber != 0)
            basePacketName = basePacketName.substr(0, basePacketName.find("-frag"));
        std::string packetName = basePacketName + "-frag" + std::to_string(fragmentNumber);
        packet->setName(packetName.c_str());
        packet->removeTagIfPresent<PacketProtocolTag>();
        // remaining part
        std::string remainingPacketName = basePacketName + "-frag" + std::to_string(fragmentNumber + 1);
        const auto& remainingData = packet->removeAtBack(packet->getDataLength() - preemptedLength);
        remainingPacket = new Packet(remainingPacketName.c_str(), remainingData);
        remainingPacket->copyTags(*packet);
        const auto& remainingPacketFragmentTag = remainingPacket->getTagForUpdate<FragmentTag>();
        remainingPacketFragmentTag->setFirstFragment(false);
        remainingPacketFragmentTag->setLastFragment(true);
        remainingPacketFragmentTag->setFragmentNumber(fragmentNumber + 1);
    }
    handlePacketProcessed(packet);
    animatePullPacketEnd(packet, outputGate, streamedPacket->getId());
    streamedPacket = nullptr;
    updateDisplayString();
    return packet;
}

Packet *PreemptableStreamer::pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    streamDatarate = datarate;
    EV_INFO << "Progressing streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    auto packet = streamedPacket->dup();
    animatePullPacketProgress(packet, outputGate, streamDatarate, position, extraProcessableLength, streamedPacket->getId());
    updateDisplayString();
    return packet;
}

void PreemptableStreamer::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collector != nullptr && !isStreaming())
        collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
}

void PreemptableStreamer::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketConfirmation");
    if (collector != nullptr)
        collector->handlePullPacketProcessed(packet, gate, successful);
}

} // namespace inet

