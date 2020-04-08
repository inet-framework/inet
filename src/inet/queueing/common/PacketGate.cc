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
#include "inet/queueing/common/PacketGate.h"

namespace inet {
namespace queueing {

Define_Module(PacketGate);

void PacketGate::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        isOpen_ = par("initiallyOpen");
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        collector = findConnectedModule<IActivePacketSink>(outputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        changeTimer = new cMessage("ChangeTimer");
        double openTime = par("openTime");
        if (!std::isnan(openTime))
            changeTimes.push_back(openTime);
        double closeTime = par("closeTime");
        if (!std::isnan(closeTime))
            changeTimes.push_back(closeTime);
        const char *changeTimesAsString = par("changeTimes");
        cStringTokenizer tokenizer(changeTimesAsString);
        while (tokenizer.hasMoreTokens())
            changeTimes.push_back(SimTime::parse(tokenizer.nextToken()));

    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
        if (changeIndex < (int)changeTimes.size())
            scheduleChangeTimer();
    }
}

void PacketGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        processChangeTimer();
        if (changeIndex < (int)changeTimes.size())
            scheduleChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void PacketGate::scheduleChangeTimer()
{
    ASSERT(0 <= changeIndex && changeIndex < (int)changeTimes.size());
    scheduleAt(changeTimes[changeIndex], changeTimer);
    changeIndex++;
}

void PacketGate::processChangeTimer()
{
    if (isOpen_)
        close();
    else
        open();
}

void PacketGate::open()
{
    ASSERT(!isOpen_);
    EV_DEBUG << "Opening gate.\n";
    isOpen_ = true;
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate);
    if (collector != nullptr)
        collector->handleCanPullPacket(outputGate);
}

void PacketGate::close()
{
    ASSERT(isOpen_);
    EV_DEBUG << "Closing gate.\n";
    isOpen_ = false;
}

bool PacketGate::canPushSomePacket(cGate *gate) const
{
    return isOpen_ && consumer->canPushSomePacket(outputGate->getPathStartGate());
}

bool PacketGate::canPushPacket(Packet *packet, cGate *gate) const
{
    return isOpen_ && consumer->canPushPacket(packet, outputGate->getPathStartGate());
}

void PacketGate::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    if (!isOpen_)
        throw cRuntimeError("Cannot push packet when the gate is closed");
    EV_INFO << "Passing packet " << packet->getName() << "." << endl;
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    updateDisplayString();
}

bool PacketGate::canPullSomePacket(cGate *gate) const
{
    return isOpen_ && provider->canPullSomePacket(inputGate->getPathStartGate());
}

Packet *PacketGate::canPullPacket(cGate *gate) const
{
    return isOpen_ ? provider->canPullPacket(inputGate->getPathStartGate()) : nullptr;
}

Packet *PacketGate::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    if (!isOpen_)
        throw cRuntimeError("Cannot pull packet when the gate is closed");
    auto packet = provider->pullPacket(inputGate->getPathStartGate());
    take(packet);
    EV_INFO << "Passing packet " << packet->getName() << "." << endl;
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    updateDisplayString();
    animateSend(packet, outputGate);
    return packet;
}

void PacketGate::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (isOpen_ && producer != nullptr)
        producer->handleCanPushPacket(inputGate);
}

void PacketGate::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (isOpen_ && collector != nullptr)
        collector->handleCanPullPacket(outputGate);
}

} // namespace queueing
} // namespace inet

