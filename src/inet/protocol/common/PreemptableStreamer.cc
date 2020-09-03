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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocol/common/PreemptableStreamer.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(PreemptableStreamer);

PreemptableStreamer::~PreemptableStreamer()
{
    delete streamedPacket;
    cancelAndDelete(endStreamingTimer);
}

void PreemptableStreamer::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
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
        endStreamingTimer = new cMessage("EndStreamingTimer");
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
    auto packetLength = streamedPacket->getTotalLength();
    EV_INFO << "Ending streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer);
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
    streamedPacket = packet->dup();
    streamedPacket->setOrigPacketId(packet->getId());
    EV_INFO << "Starting streaming" << EV_FIELD(packet) << EV_ENDL;
    pushOrSendPacketStart(packet, outputGate, consumer, datarate);
    if (std::isnan(streamDatarate.get()))
        endStreaming();
    else
        scheduleAfter(s(streamedPacket->getTotalLength() / streamDatarate).get(), endStreamingTimer);
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
    auto packet = remainingPacket == nullptr ? provider->pullPacket(inputGate->getPathStartGate()) : remainingPacket;
    remainingPacket = nullptr;
    auto fragmentTag = packet->findTagForUpdate<FragmentTag>();
    if (fragmentTag == nullptr) {
        fragmentTag = packet->addTag<FragmentTag>();
        fragmentTag->setFirstFragment(true);
        fragmentTag->setLastFragment(true);
        fragmentTag->setFragmentNumber(0);
        fragmentTag->setNumFragments(-1);
    }
    streamStart = simTime();
    streamedPacket = packet->dup();
    streamedPacket->setOrigPacketId(packet->getId());
    EV_INFO << "Starting streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    updateDisplayString();
    return packet;
}

Packet *PreemptableStreamer::pullPacketEnd(cGate *gate)
{
    Enter_Method("pullPacketEnd");
    EV_INFO << "Ending streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    auto packet = streamedPacket;
    b pulledLength = streamDatarate * s((simTime() - streamStart).dbl());
    b preemptedLength = roundingLength * ((pulledLength + roundingLength - b(1)) / roundingLength);
    if (preemptedLength < minPacketLength)
        preemptedLength = minPacketLength;
    if (preemptedLength + minPacketLength <= packet->getTotalLength()) {
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
        const auto& remainingData = packet->removeAtBack(packet->getTotalLength() - preemptedLength);
        remainingPacket = new Packet(remainingPacketName.c_str(), remainingData);
        remainingPacket->copyTags(*packet);
        const auto& remainingPacketFragmentTag = remainingPacket->getTagForUpdate<FragmentTag>();
        remainingPacketFragmentTag->setFirstFragment(false);
        remainingPacketFragmentTag->setLastFragment(true);
        remainingPacketFragmentTag->setFragmentNumber(fragmentNumber + 1);
    }
    streamedPacket = nullptr;
    handlePacketProcessed(packet);
    updateDisplayString();
    return packet;
}

Packet *PreemptableStreamer::pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    EV_INFO << "Progressing streaming" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    updateDisplayString();
    return streamedPacket->dup();
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

