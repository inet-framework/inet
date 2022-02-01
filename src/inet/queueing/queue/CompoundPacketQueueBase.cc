//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/queue/CompoundPacketQueueBase.h"

#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

Define_Module(CompoundPacketQueueBase);

void CompoundPacketQueueBase::initialize(int stage)
{
    PacketQueueBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetCapacity = par("packetCapacity");
        dataCapacity = b(par("dataCapacity"));
        consumer = check_and_cast<IPassivePacketSink *>(inputGate->getPathEndGate()->getOwnerModule());
        provider = check_and_cast<IPassivePacketSource *>(outputGate->getPathStartGate()->getOwnerModule());
        collection = check_and_cast<IPacketCollection *>(provider);
        packetDropperFunction = createDropperFunction(par("dropperClass"));
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

IPacketDropperFunction *CompoundPacketQueueBase::createDropperFunction(const char *dropperClass) const
{
    if (strlen(dropperClass) == 0)
        return nullptr;
    else
        return check_and_cast<IPacketDropperFunction *>(createOne(dropperClass));
}

bool CompoundPacketQueueBase::isOverloaded() const
{
    return (packetCapacity != -1 && getNumPackets() > packetCapacity) ||
           (dataCapacity != b(-1) && getTotalLength() > dataCapacity);
}

void CompoundPacketQueueBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    emit(packetPushStartedSignal, packet);
    auto packetClone = packet->dup(); // TODO delete this code if the original packet can be emitted in packetPushEndedSignal
    EV_INFO << "Pushing packet" << EV_FIELD(packet) << EV_ENDL;
    consumer->pushPacket(packet, inputGate->getPathEndGate());
    if (packetDropperFunction != nullptr) {
        while (isOverloaded()) {
            auto packet = packetDropperFunction->selectPacket(this);
            EV_INFO << "Dropping packet" << EV_FIELD(packet) << EV_ENDL;
            removePacket(packet);
            dropPacket(packet, QUEUE_OVERFLOW);
        }
    }
    ASSERT(!isOverloaded());
    // TODO pass packet instead of packetClone if not deleted when weak ptrs are available
    emit(packetPushEndedSignal, packetClone);
    delete packetClone;
    updateDisplayString();
}

Packet *CompoundPacketQueueBase::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = provider->pullPacket(outputGate->getPathStartGate());
    take(packet);
    emit(packetPulledSignal, packet);
    updateDisplayString();
    return packet;
}

void CompoundPacketQueueBase::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    collection->removePacket(packet);
    emit(packetRemovedSignal, packet);
    updateDisplayString();
}

void CompoundPacketQueueBase::removeAllPackets()
{
    Enter_Method("removeAllPacket");
    collection->removeAllPackets();
    updateDisplayString();
}

bool CompoundPacketQueueBase::canPushSomePacket(cGate *gate) const
{
    if (packetDropperFunction)
        return true;
    if (getMaxNumPackets() != -1 && getNumPackets() >= getMaxNumPackets())
        return false;
    if (getMaxTotalLength() != b(-1) && getTotalLength() >= getMaxTotalLength())
        return false;
    return true;
}

bool CompoundPacketQueueBase::canPushPacket(Packet *packet, cGate *gate) const
{
    if (packetDropperFunction)
        return true;
    if (getMaxNumPackets() != -1 && getNumPackets() >= getMaxNumPackets())
        return false;
    if (getMaxTotalLength() != b(-1) && getMaxTotalLength() - getTotalLength() < packet->getDataLength())
        return false;
    return true;
}

void CompoundPacketQueueBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == packetDroppedSignal)
        numDroppedPackets++;
    else if (signal == packetCreatedSignal)
        numCreatedPackets++;
    else
        throw cRuntimeError("Unknown signal");
    updateDisplayString();
}

} // namespace queueing
} // namespace inet

