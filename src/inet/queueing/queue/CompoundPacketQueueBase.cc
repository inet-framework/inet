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
        consumer.reference(inputGate, true, 1);
        provider.reference(outputGate, true, -1);
        collection = check_and_cast<IPacketCollection *>(provider.get());
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

void CompoundPacketQueueBase::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    cNamedObject packetPushStartedDetails("atomicOperationStarted");
    emit(packetPushStartedSignal, packet, &packetPushStartedDetails);
    animatePushPacket(packet, inputGate, consumer.getReferencedGate());
    EV_INFO << "Pushing packet" << EV_FIELD(packet) << EV_ENDL;
    consumer->pushPacket(packet, consumer.getReferencedGate());
    if (packetDropperFunction != nullptr) {
        while (isOverloaded()) {
            auto packet = packetDropperFunction->selectPacket(this);
            EV_INFO << "Dropping packet" << EV_FIELD(packet) << EV_ENDL;
            removePacket(packet);
            dropPacket(packet, QUEUE_OVERFLOW);
        }
    }
    ASSERT(!isOverloaded());
    cNamedObject packetPushEndedDetails("atomicOperationEnded");
    emit(packetPushEndedSignal, nullptr, &packetPushEndedDetails);
    updateDisplayString();
}

Packet *CompoundPacketQueueBase::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = provider->pullPacket(provider.getReferencedGate());
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

bool CompoundPacketQueueBase::canPushSomePacket(const cGate *gate) const
{
    if (packetDropperFunction)
        return true;
    if (getMaxNumPackets() != -1 && getNumPackets() >= getMaxNumPackets())
        return false;
    if (getMaxTotalLength() != b(-1) && getTotalLength() >= getMaxTotalLength())
        return false;
    return true;
}

bool CompoundPacketQueueBase::canPushPacket(Packet *packet, const cGate *gate) const
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

