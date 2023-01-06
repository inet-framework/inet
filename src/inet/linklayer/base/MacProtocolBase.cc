//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/base/MacProtocolBase.h"

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {

MacProtocolBase::MacProtocolBase()
{
}

MacProtocolBase::~MacProtocolBase()
{
    delete currentTxFrame;
}

MacAddress MacProtocolBase::parseMacAddressParameter(const char *addrstr)
{
    MacAddress address;

    if (!strcmp(addrstr, "auto"))
        // assign automatic address
        address = MacAddress::generateAutoAddress();
    else
        address.setAddress(addrstr);

    return address;
}

void MacProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        upperLayerInGateId = findGate("upperLayerIn");
        upperLayerOutGateId = findGate("upperLayerOut");
        lowerLayerInGateId = findGate("lowerLayerIn");
        lowerLayerOutGateId = findGate("lowerLayerOut");
        currentTxFrame = nullptr;
        hostModule = findContainingNode(this);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION)
        registerInterface();
}

void MacProtocolBase::registerInterface()
{
    ASSERT(networkInterface == nullptr);
    networkInterface = getContainingNicModule(this);
    configureNetworkInterface();
}

void MacProtocolBase::sendUp(cMessage *message)
{
    if (message->isPacket())
        emit(packetSentToUpperSignal, message);
    send(message, upperLayerOutGateId);
}

void MacProtocolBase::sendDown(cMessage *message)
{
    if (message->isPacket())
        emit(packetSentToLowerSignal, message);
    send(message, lowerLayerOutGateId);
}

bool MacProtocolBase::isUpperMessage(cMessage *message) const
{
    return message->getArrivalGateId() == upperLayerInGateId;
}

bool MacProtocolBase::isLowerMessage(cMessage *message) const
{
    return message->getArrivalGateId() == lowerLayerInGateId;
}

void MacProtocolBase::handleMessageWhenDown(cMessage *msg)
{
    if (!msg->isSelfMessage() && msg->getArrivalGateId() == lowerLayerInGateId) {
        EV << "Interface is turned off, dropping packet\n";
        delete msg;
    }
    else
        LayeredProtocolBase::handleMessageWhenDown(msg);
}

void MacProtocolBase::deleteCurrentTxFrame()
{
    delete currentTxFrame;
    currentTxFrame = nullptr;
}

void MacProtocolBase::dropCurrentTxFrame(PacketDropDetails& details)
{
    emit(packetDroppedSignal, currentTxFrame, &details);
    delete currentTxFrame;
    currentTxFrame = nullptr;
}

void MacProtocolBase::flushQueue(PacketDropDetails& details)
{
    // code would look slightly nicer with a pop() function that returns nullptr if empty
    if (txQueue)
        while (canDequeuePacket()) {
            auto packet = dequeuePacket();
            emit(packetDroppedSignal, packet, &details); // FIXME this signal lumps together packets from the network and packets from higher layers! separate them
            delete packet;
        }
}

void MacProtocolBase::clearQueue()
{
    if (txQueue)
        while (canDequeuePacket())
            delete dequeuePacket();
}

void MacProtocolBase::handleStartOperation(LifecycleOperation *operation)
{
    networkInterface->setState(NetworkInterface::State::UP);
    networkInterface->setCarrier(true);
}

void MacProtocolBase::handleStopOperation(LifecycleOperation *operation)
{
    PacketDropDetails details;
    details.setReason(INTERFACE_DOWN);
    if (currentTxFrame)
        dropCurrentTxFrame(details);
    flushQueue(details);

    networkInterface->setCarrier(false);
    networkInterface->setState(NetworkInterface::State::DOWN);
}

void MacProtocolBase::handleCrashOperation(LifecycleOperation *operation)
{
    deleteCurrentTxFrame();
    clearQueue();

    networkInterface->setCarrier(false);
    networkInterface->setState(NetworkInterface::State::DOWN);
}

void MacProtocolBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
}


queueing::IPacketQueue *MacProtocolBase::getQueue(cGate *gate) const
{
    // TODO use findConnectedModule() when the function is updated
    for (auto g = gate->getPreviousGate(); g != nullptr; g = g->getPreviousGate()) {
        if (g->getType() == cGate::OUTPUT) {
            auto m = dynamic_cast<queueing::IPacketQueue *>(g->getOwnerModule());
            if (m)
                return m;
        }
    }
    throw cRuntimeError("Gate %s is not connected to a module of type queueing::IPacketQueue (did you use OmittedPacketQueue as queue type?)", gate->getFullPath().c_str());
}

bool MacProtocolBase::canDequeuePacket() const
{
    return txQueue && txQueue->canPullSomePacket(gate(upperLayerInGateId)->getPathStartGate());
}

Packet *MacProtocolBase::dequeuePacket()
{
    Packet *packet = txQueue->dequeuePacket();
    take(packet);
    packet->setArrival(getId(), upperLayerInGateId, simTime());
    emit(packetReceivedFromUpperSignal, packet);
    return packet;
}

} // namespace inet

