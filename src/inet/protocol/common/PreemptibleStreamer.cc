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
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocol/common/PreemptibleStreamer.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(PreemptibleStreamer);

void PreemptibleStreamer::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minPacketLength = b(par("minPacketLength"));
        roundingLength = b(par("roundingLength"));
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        collector = findConnectedModule<IActivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PreemptibleStreamer::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

bool PreemptibleStreamer::canPushSomePacket(cGate *gate) const
{
    return !isStreaming() && consumer->canPushSomePacket(outputGate->getPathEndGate());
}

bool PreemptibleStreamer::canPushPacket(Packet *packet, cGate *gate) const
{
    return !isStreaming() && consumer->canPushPacket(packet, outputGate->getPathEndGate());
}

void PreemptibleStreamer::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    ASSERT(!isStreaming());
    take(packet);
    streamedPacket = packet;
    EV_INFO << "Starting streaming packet " << packet->getName() << "." << std::endl;
    pushOrSendPacketStart(packet->dup(), outputGate, consumer, datarate);
    EV_INFO << "Ending streaming packet " << packet->getName() << "." << std::endl;
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer, datarate);
    streamedPacket = nullptr;
    handlePacketProcessed(packet);
    updateDisplayString();
}

void PreemptibleStreamer::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
}

void PreemptibleStreamer::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

bool PreemptibleStreamer::canPullSomePacket(cGate *gate) const
{
    return !isStreaming() && provider->canPullSomePacket(inputGate->getPathStartGate());
}

Packet *PreemptibleStreamer::canPullPacket(cGate *gate) const
{
    return isStreaming() ? nullptr : provider->canPullPacket(inputGate->getPathStartGate());
}

Packet *PreemptibleStreamer::pullPacketStart(cGate *gate, bps datarate)
{
    Enter_Method("pullPacketStart");
    streamStart = simTime();
    streamedPacket = remainingPacket == nullptr ? provider->pullPacket(inputGate->getPathStartGate()) : remainingPacket;
    remainingPacket = nullptr;
    auto fragmentTag = streamedPacket->findTag<FragmentTag>();
    if (fragmentTag == nullptr) {
        fragmentTag = streamedPacket->addTag<FragmentTag>();
        fragmentTag->setFirstFragment(true);
        fragmentTag->setLastFragment(true);
        fragmentTag->setFragmentNumber(0);
        fragmentTag->setNumFragments(-1);
    }
    EV_INFO << "Starting streaming packet " << streamedPacket->getName() << "." << std::endl;
    updateDisplayString();
    return streamedPacket->dup();
}

Packet *PreemptibleStreamer::pullPacketEnd(cGate *gate, bps datarate)
{
    Enter_Method("pullPacketEnd");
    EV_INFO << "Ending streaming packet " << streamedPacket->getName() << "." << std::endl;
    auto packet = streamedPacket;
    b pulledLength = datarate * s((simTime() - streamStart).dbl());
    b preemptedLength = roundingLength * ((pulledLength + roundingLength - b(1)) / roundingLength);
    if (preemptedLength < minPacketLength)
        preemptedLength = minPacketLength;
    if (preemptedLength + minPacketLength <= packet->getTotalLength()) {
        std::string name = std::string(packet->getName()) + "-frag";
        // already pulled part
        packet->setName(name.c_str());
        packet->removeTagIfPresent<PacketProtocolTag>();
        const auto& remainingData = packet->removeAtBack(packet->getTotalLength() - preemptedLength);
        FragmentTag *fragmentTag = packet->getTag<FragmentTag>();
        fragmentTag->setLastFragment(false);
        // remaining part
        remainingPacket = new Packet(name.c_str(), remainingData);
        remainingPacket->copyTags(*packet);
        FragmentTag *remainingPacketFragmentTag = remainingPacket->getTag<FragmentTag>();
        remainingPacketFragmentTag->setFirstFragment(false);
        remainingPacketFragmentTag->setLastFragment(true);
        remainingPacketFragmentTag->setFragmentNumber(fragmentTag->getFragmentNumber() + 1);
    }
    streamedPacket = nullptr;
    handlePacketProcessed(packet);
    updateDisplayString();
    return packet;
}

Packet *PreemptibleStreamer::pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    EV_INFO << "Progressing streaming packet " << streamedPacket->getName() << "." << std::endl;
    updateDisplayString();
    return streamedPacket->dup();
}

b PreemptibleStreamer::getPullPacketProcessedLength(Packet *packet, cGate *gate)
{
    return streamedPacket->getTotalLength();
}

void PreemptibleStreamer::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (collector != nullptr && !isStreaming())
        collector->handleCanPullPacket(outputGate->getPathEndGate());
}

void PreemptibleStreamer::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketConfirmation");
    if (collector != nullptr)
        collector->handlePullPacketProcessed(packet, gate, successful);
}

} // namespace inet

