//
// (C) 2013 Opensim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Author: Andras Varga (andras@omnetpp.org)
//

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/base/MacProtocolBase.h"

namespace inet {

MacProtocolBase::MacProtocolBase()
{
}

MacProtocolBase::~MacProtocolBase()
{
    delete currentTxFrame;
    if (hostModule)
        hostModule->unsubscribe(interfaceDeletedSignal, this);
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
        hostModule = findContainingNode(this);
        if (hostModule)
            hostModule->subscribe(interfaceDeletedSignal, this);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION)
        registerInterface();
}

void MacProtocolBase::registerInterface()
{
    ASSERT(interfaceEntry == nullptr);
    interfaceEntry = getContainingNicModule(this);
    configureInterfaceEntry();
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

bool MacProtocolBase::isUpperMessage(cMessage *message)
{
    return message->getArrivalGateId() == upperLayerInGateId;
}

bool MacProtocolBase::isLowerMessage(cMessage *message)
{
    return message->getArrivalGateId() == lowerLayerInGateId;
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

void MacProtocolBase::popTxQueue()
{
    if (currentTxFrame != nullptr)
        throw cRuntimeError("Model error: incomplete transmission exists");
    ASSERT(txQueue != nullptr);
    currentTxFrame = txQueue->dequeuePacket();
    take(currentTxFrame);
}

void MacProtocolBase::flushQueue(PacketDropDetails& details)
{
    // code would look slightly nicer with a pop() function that returns nullptr if empty
    if (txQueue)
        while (!txQueue->isEmpty()) {
            auto packet = txQueue->dequeuePacket();
            emit(packetDroppedSignal, packet, &details); //FIXME this signal lumps together packets from the network and packets from higher layers! separate them
            delete packet;
        }
}

void MacProtocolBase::clearQueue()
{
    if (txQueue)
        while (!txQueue->isEmpty())
            delete txQueue->dequeuePacket();
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

void MacProtocolBase::handleStartOperation(LifecycleOperation *operation)
{
    interfaceEntry->setState(InterfaceEntry::State::UP);
    interfaceEntry->setCarrier(true);
}

void MacProtocolBase::handleStopOperation(LifecycleOperation *operation)
{
    PacketDropDetails details;
    details.setReason(INTERFACE_DOWN);
    if (currentTxFrame)
        dropCurrentTxFrame(details);
    flushQueue(details);
    interfaceEntry->setCarrier(false);
    interfaceEntry->setState(InterfaceEntry::State::DOWN);
}

void MacProtocolBase::handleCrashOperation(LifecycleOperation *operation)
{
    deleteCurrentTxFrame();
    clearQueue();
    interfaceEntry->setCarrier(false);
    interfaceEntry->setState(InterfaceEntry::State::DOWN);
}

void MacProtocolBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    if (signalID == interfaceDeletedSignal) {
        if (interfaceEntry == check_and_cast<const InterfaceEntry *>(obj))
            interfaceEntry = nullptr;
    }
}

} // namespace inet

