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

#include "inet/common/Simsignals.h"
#include "inet/queueing/queue/CompoundPacketQueue.h"

namespace inet {
namespace queueing {

Define_Module(CompoundPacketQueue);

void CompoundPacketQueue::initialize(int stage)
{
    PacketQueueBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetCapacity = par("packetCapacity");
        dataCapacity = b(par("dataCapacity"));
        consumer = check_and_cast<IPassivePacketSink *>(inputGate->getPathEndGate()->getOwnerModule());
        provider = check_and_cast<IPassivePacketSource *>(outputGate->getPathStartGate()->getOwnerModule());
        collection = check_and_cast<IPacketCollection *>(provider);
        subscribe(packetPushedSignal, this);
        subscribe(packetPulledSignal, this);
        subscribe(packetRemovedSignal, this);
        subscribe(packetDroppedSignal, this);
        subscribe(packetCreatedSignal, this);
        WATCH(numCreatedPackets);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

void CompoundPacketQueue::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    emit(packetPushedSignal, packet);
    if ((packetCapacity != -1 && getNumPackets() >= packetCapacity) ||
        (dataCapacity != b(-1) && getTotalLength() + packet->getTotalLength() > dataCapacity))
    {
        EV_INFO << "Dropping packet " << packet->getName() << " because the queue is full." << endl;
        dropPacket(packet, QUEUE_OVERFLOW, packetCapacity);
    }
    else {
        consumer->pushPacket(packet, inputGate->getPathEndGate());
        updateDisplayString();
    }
}

Packet *CompoundPacketQueue::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = provider->pullPacket(outputGate->getPathStartGate());
    take(packet);
    emit(packetPulledSignal, packet);
    updateDisplayString();
    return packet;
}

void CompoundPacketQueue::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    collection->removePacket(packet);
    emit(packetRemovedSignal, packet);
    updateDisplayString();
}

void CompoundPacketQueue::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == packetPushedSignal) {
        Enter_Method("receivePacketPushedSignal");
    }
    else if (signal == packetPulledSignal) {
        Enter_Method("receivePacketPulledSignal");
    }
    else if (signal == packetRemovedSignal) {
        Enter_Method("receivePacketRemovedSignal");
    }
    else if (signal == packetDroppedSignal) {
        Enter_Method("receivePacketDroppedSignal");
        numDroppedPackets++;
    }
    else if (signal == packetCreatedSignal) {
        Enter_Method("receivePacketCreatedSignal");
        numCreatedPackets++;
    }
    else
        throw cRuntimeError("Unknown signal");
    updateDisplayString();
}

} // namespace queueing
} // namespace inet

