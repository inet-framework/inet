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
        producer.reference(inputGate, false);
        provider.reference(inputGate, false);
        consumer.reference(outputGate, false);
        collector.reference(outputGate, false);
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
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer.getReferencedGate(), consumer, streamedPacket->getId());
    streamDatarate = bps(NaN);
    streamedPacket = nullptr;
    numProcessedPackets++;
    processedTotalLength += packetLength;
    updateDisplayString();
}

bool PreemptableStreamer::canPushSomePacket(const cGate *gate) const
{
    return !isStreaming() && consumer->canPushSomePacket(consumer.getReferencedGate());
}

bool PreemptableStreamer::canPushPacket(Packet *packet, const cGate *gate) const
{
    return !isStreaming() && consumer->canPushPacket(packet, consumer.getReferencedGate());
}

void PreemptableStreamer::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    ASSERT(!isStreaming());
    take(packet);
    streamDatarate = datarate;
    streamedPacket = packet;
    EV_INFO << "Starting streaming packet" << EV_FIELD(packet) << EV_ENDL;
    pushOrSendPacketStart(streamedPacket->dup(), outputGate, consumer.getReferencedGate(), consumer, datarate, streamedPacket->getId());
    if (std::isnan(streamDatarate.get()))
        endStreaming();
    else
        scheduleClockEventAfter(s(streamedPacket->getDataLength() / streamDatarate).get(), endStreamingTimer);
}

void PreemptableStreamer::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(producer.getReferencedGate());
}

void PreemptableStreamer::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, producer.getReferencedGate(), successful);
}

bool PreemptableStreamer::canPullSomePacket(const cGate *gate) const
{
    return !isStreaming() && (remainingPacket != nullptr || provider->canPullSomePacket(provider.getReferencedGate()));
}

Packet *PreemptableStreamer::canPullPacket(const cGate *gate) const
{
    return isStreaming() ? nullptr : remainingPacket != nullptr ? remainingPacket : provider->canPullPacket(provider.getReferencedGate());
}

Packet *PreemptableStreamer::pullPacketStart(const cGate *gate, bps datarate)
{
    Enter_Method("pullPacketStart");
    streamDatarate = datarate;
    if (remainingPacket == nullptr) {
        streamedPacket = provider->pullPacket(provider.getReferencedGate());
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
    animatePullPacketStart(packet, outputGate, findConnectedGate<IActivePacketSink>(outputGate), streamDatarate, streamedPacket->getId());
    updateDisplayString();
    return packet;
}

Packet *PreemptableStreamer::pullPacketEnd(const cGate *gate)
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
    animatePullPacketEnd(packet, outputGate, findConnectedGate<IActivePacketSink>(outputGate), streamedPacket->getId());
    streamedPacket = nullptr;
    updateDisplayString();
    return packet;
}

Packet *PreemptableStreamer::pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    streamDatarate = datarate;
    EV_INFO << "Progressing streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    auto packet = streamedPacket->dup();
    animatePullPacketProgress(packet, outputGate, findConnectedGate<IActivePacketSink>(outputGate), streamDatarate, position, extraProcessableLength, streamedPacket->getId());
    updateDisplayString();
    return packet;
}

void PreemptableStreamer::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collector != nullptr && !isStreaming())
        collector->handleCanPullPacketChanged(collector.getReferencedGate());
}

void PreemptableStreamer::handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketConfirmation");
    if (collector != nullptr)
        collector->handlePullPacketProcessed(packet, gate, successful);
}

} // namespace inet

