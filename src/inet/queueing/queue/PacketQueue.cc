//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/queue/PacketQueue.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/queueing/function/PacketComparatorFunction.h"
#include "inet/queueing/function/PacketDropperFunction.h"

namespace inet {
namespace queueing {

Define_Module(PacketQueue);

void PacketQueue::initialize(int stage)
{
    PacketQueueBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        queue.setName("storage");
        producer.reference(inputGate, false);
        collector.reference(outputGate, false);
        packetCapacity = par("packetCapacity");
        dataCapacity = b(par("dataCapacity"));
        buffer = findModuleFromPar<IPacketBuffer>(par("bufferModule"), this);
        packetComparatorFunction = createComparatorFunction(par("comparatorClass"));
        if (packetComparatorFunction != nullptr)
            queue.setup(packetComparatorFunction);
        packetDropperFunction = createDropperFunction(par("dropperClass"));
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
        if (producer != nullptr)
            producer.handleCanPushPacketChanged();
    }
}

cGate *PacketQueue::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

IPacketDropperFunction *PacketQueue::createDropperFunction(const char *dropperClass) const
{
    if (strlen(dropperClass) == 0)
        return nullptr;
    else
        return check_and_cast<IPacketDropperFunction *>(createOne(dropperClass));
}

IPacketComparatorFunction *PacketQueue::createComparatorFunction(const char *comparatorClass) const
{
    if (strlen(comparatorClass) == 0)
        return nullptr;
    else
        return check_and_cast<IPacketComparatorFunction *>(createOne(comparatorClass));
}

bool PacketQueue::isOverloaded() const
{
    return (packetCapacity != -1 && getNumPackets() > packetCapacity) ||
           (dataCapacity != b(-1) && getTotalLength() > dataCapacity);
}

int PacketQueue::getNumPackets() const
{
    return queue.getLength();
}

Packet *PacketQueue::getPacket(int index) const
{
    if (index < 0 || index >= queue.getLength())
        throw cRuntimeError("index %i out of range", index);
    return check_and_cast<Packet *>(queue.get(index));
}

void PacketQueue::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    cNamedObject packetPushStartedDetails("atomicOperationStarted");
    emit(packetPushStartedSignal, packet, &packetPushStartedDetails);
    EV_INFO << "Pushing packet" << EV_FIELD(packet) << EV_ENDL;
    queue.insert(packet);
    if (buffer != nullptr) {
        buffer->addPacket(packet);
        if (isOverloaded())
            throw cRuntimeError("Queue is overloaded while using a packet buffer");
    }
    else if (packetDropperFunction != nullptr) {
        std::vector<Packet *> droppedPackets;
        while (isOverloaded()) {
            auto packet = packetDropperFunction->selectPacket(this);
            EV_INFO << "Dropping packet" << EV_FIELD(packet) << EV_ENDL;
            queue.remove(packet);
            droppedPackets.push_back(packet);
        }
        for (auto packet : droppedPackets) {
            notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::DROPPED);
            dropPacket(packet, QUEUE_OVERFLOW);
        }
    }
    else if (isOverloaded())
        throw cRuntimeError("Queue is overloaded without a packet dropper");
    if (collector != nullptr && getNumPackets() != 0)
        collector.handleCanPullPacketChanged();
    cNamedObject packetPushEndedDetails("atomicOperationEnded");
    emit(packetPushEndedSignal, nullptr, &packetPushEndedDetails);
}

Packet *PacketQueue::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = check_and_cast<Packet *>(queue.front());
    EV_INFO << "Pulling packet" << EV_FIELD(packet) << EV_ENDL;
    if (buffer != nullptr) {
        queue.remove(packet);
        buffer->removePacket(packet);
    }
    else
        queue.pop();
    notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::DEQUEUED);
    recordPacketDequeued(packet);
    if (collector != nullptr)
        animatePullPacket(packet, outputGate, collector.getReferencedGate());
    return packet;
}

Packet *PacketQueue::findPacket(const PacketPredicate& predicate) const
{
    for (int i = 0; i < queue.getLength(); i++) {
        auto packet = check_and_cast<Packet *>(queue.get(i));
        if (predicate(packet))
            return packet;
    }
    return nullptr;
}

Packet *PacketQueue::dequeuePacket(const PacketPredicate& predicate)
{
    Enter_Method("dequeuePacket");
    auto packet = findPacket(predicate);
    if (packet == nullptr)
        return nullptr;
    EV_INFO << "Dequeuing packet" << EV_FIELD(packet) << EV_ENDL;
    queue.remove(packet);
    if (buffer != nullptr)
        buffer->removePacket(packet);
    notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::DEQUEUED);
    recordPacketDequeued(packet);
    if (collector != nullptr)
        animatePullPacket(packet, outputGate, collector.getReferencedGate());
    drop(packet);
    return packet;
}

void PacketQueue::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    EV_INFO << "Removing packet" << EV_FIELD(packet) << EV_ENDL;
    queue.remove(packet);
    if (buffer != nullptr)
        buffer->removePacket(packet);
    notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::REMOVED);
    emit(packetRemovedSignal, packet);
}

void PacketQueue::removeAllPackets()
{
    Enter_Method("removeAllPackets");
    EV_INFO << "Removing all packets" << EV_ENDL;
    std::vector<Packet *> packets;
    while (!queue.isEmpty())
        packets.push_back(check_and_cast<Packet *>(queue.pop()));
    if (buffer != nullptr)
        for (auto packet : packets)
            buffer->removePacket(packet);
    for (auto packet : packets) {
        notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::REMOVED);
        emit(packetRemovedSignal, packet);
        delete packet;
    }
}

bool PacketQueue::canPushSomePacket(const cGate *gate) const
{
    if (packetDropperFunction)
        return true;
    if (getMaxNumPackets() != -1 && getNumPackets() >= getMaxNumPackets())
        return false;
    if (getMaxTotalLength() != b(-1) && getTotalLength() >= getMaxTotalLength())
        return false;
    return true;
}

bool PacketQueue::canPushPacket(Packet *packet, const cGate *gate) const
{
    if (packetDropperFunction)
        return true;
    if (getMaxNumPackets() != -1 && getNumPackets() >= getMaxNumPackets())
        return false;
    if (getMaxTotalLength() != b(-1) && getMaxTotalLength() - getTotalLength() < packet->getDataLength())
        return false;
    return true;
}

void PacketQueue::handlePacketRemoved(Packet *packet)
{
    Enter_Method("handlePacketRemoved");
    if (queue.contains(packet)) {
        EV_INFO << "Removing packet" << EV_FIELD(packet) << EV_ENDL;
        queue.remove(packet);
        emit(packetRemovedSignal, packet);
        notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::REMOVED);
    }
}

void PacketQueue::handlePacketDropping(Packet *packet)
{
    Enter_Method("handlePacketDropping");
    if (queue.contains(packet)) {
        EV_INFO << "Removing packet" << EV_FIELD(packet) << EV_ENDL;
        queue.remove(packet);
        emit(packetRemovedSignal, packet);
    }
}

void PacketQueue::handlePacketDropped(Packet *packet)
{
    Enter_Method("handlePacketDropped");
    notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::DROPPED);
}

} // namespace queueing
} // namespace inet
