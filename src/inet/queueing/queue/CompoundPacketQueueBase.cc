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

class ScopedPacketRemoval
{
  protected:
    Packet *&packetBeingRemoved;
    Packet *previousPacket;

  public:
    ScopedPacketRemoval(Packet *&packetBeingRemoved, Packet *packet) :
        packetBeingRemoved(packetBeingRemoved), previousPacket(packetBeingRemoved)
    {
        packetBeingRemoved = packet;
    }

    ~ScopedPacketRemoval() { packetBeingRemoved = previousPacket; }
};

void CompoundPacketQueueBase::initialize(int stage)
{
    PacketQueueBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetCapacity = par("packetCapacity");
        dataCapacity = b(par("dataCapacity"));
        consumer.reference(inputGate, true, 1);
        provider.reference(outputGate, true, -1);
        collection = check_and_cast<IPacketCollection *>(provider.get());
        packetExtractor = check_and_cast<IPacketExtractor *>(provider.get());
        // Observe the nearest queue on every descendant branch. Nested
        // compound queues forward their own frontier, so stopping at a queue
        // avoids duplicate notifications while traversing non-queue wrappers.
        registerQueueFrontier(this);
        packetDropperFunction = createDropperFunction(par("dropperClass"));
        subscribe(packetDroppedSignal, this);
        subscribe(packetCreatedSignal, this);
        WATCH(numCreatedPackets);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void CompoundPacketQueueBase::registerQueueFrontier(cModule *module)
{
    for (cModule::SubmoduleIterator it(module); !it.end(); it++) {
        auto childModule = *it;
        auto childQueue = dynamic_cast<IPacketQueue *>(childModule);
        if (childQueue != nullptr) {
            childQueues.push_back(childQueue);
            childQueue->addPacketCallback(this);
        }
        else
            registerQueueFrontier(childModule);
    }
}

void CompoundPacketQueueBase::finish()
{
    unregisterChildQueueCallbacks();
    PacketQueueBase::finish();
}

void CompoundPacketQueueBase::preDelete(cComponent *root)
{
    unregisterChildQueueCallbacks();
    PacketQueueBase::preDelete(root);
}

void CompoundPacketQueueBase::unregisterChildQueueCallbacks()
{
    for (auto childQueue : childQueues)
        childQueue->removePacketCallback(this);
    childQueues.clear();
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
    consumer.pushPacket(packet);
    if (packetDropperFunction != nullptr) {
        std::vector<Packet *> droppedPackets;
        while (isOverloaded()) {
            auto packet = packetDropperFunction->selectPacket(this);
            EV_INFO << "Dropping packet" << EV_FIELD(packet) << EV_ENDL;
            {
                ScopedPacketRemoval scopedPacketRemoval(packetBeingRemoved, packet);
                collection->removePacket(packet);
            }
            emit(packetRemovedSignal, packet);
            take(packet);
            droppedPackets.push_back(packet);
        }
        for (auto packet : droppedPackets) {
            notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::DROPPED);
            dropPacket(packet, QUEUE_OVERFLOW);
        }
    }
    ASSERT(!isOverloaded());
    cNamedObject packetPushEndedDetails("atomicOperationEnded");
    emit(packetPushEndedSignal, nullptr, &packetPushEndedDetails);
}

Packet *CompoundPacketQueueBase::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = provider.pullPacket();
    take(packet);
    notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::DEQUEUED);
    emit(packetPulledSignal, packet);
    return packet;
}

void CompoundPacketQueueBase::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    {
        ScopedPacketRemoval scopedPacketRemoval(packetBeingRemoved, packet);
        collection->removePacket(packet);
    }
    notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::REMOVED);
    emit(packetRemovedSignal, packet);
}

Packet *CompoundPacketQueueBase::findPacket(const PacketPredicate& predicate) const
{
    return packetExtractor->findPacket(predicate);
}

Packet *CompoundPacketQueueBase::dequeuePacket(const PacketPredicate& predicate)
{
    Enter_Method("dequeuePacket");
    auto packet = packetExtractor->dequeuePacket(predicate);
    if (packet == nullptr)
        return nullptr;
    take(packet);
    notifyPacketRemoved(packet, IPacketQueue::PacketRemovalReason::DEQUEUED);
    // The owning leaf/provider has already recorded queue residence. The
    // compound boundary mirrors pullPacket() and emits only its pull event.
    emit(packetPulledSignal, packet);
    drop(packet);
    return packet;
}

void CompoundPacketQueueBase::removeAllPackets()
{
    Enter_Method("removeAllPacket");
    while (getNumPackets() != 0) {
        auto packet = getPacket(0);
        removePacket(packet);
        take(packet);
        delete packet;
    }
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
}

void CompoundPacketQueueBase::handlePacketRemoved(Packet *packet, IPacketQueue::PacketRemovalReason reason)
{
    Enter_Method("handlePacketRemoved");
    if (reason == IPacketQueue::PacketRemovalReason::DROPPED ||
        (reason == IPacketQueue::PacketRemovalReason::REMOVED && packet != packetBeingRemoved))
        notifyPacketRemoved(packet, reason);
}

} // namespace queueing
} // namespace inet
