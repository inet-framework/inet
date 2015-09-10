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

#include "inet/networklayer/base/NetworkProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/NetworkProtocolCommand_m.h"

namespace inet {

NetworkProtocolBase::NetworkProtocolBase() :
    interfaceTable(nullptr)
{
}

void NetworkProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
}

void NetworkProtocolBase::handleUpperCommand(cMessage *message)
{
    if (dynamic_cast<RegisterTransportProtocolCommand *>(message)) {
        RegisterTransportProtocolCommand *command = check_and_cast<RegisterTransportProtocolCommand *>(message);
        protocolMapping.addProtocolMapping(command->getProtocol(), message->getArrivalGate()->getIndex());
        delete message;
    }
    else
        LayeredProtocolBase::handleUpperCommand(message);
}

void NetworkProtocolBase::sendUp(cMessage *message, int transportProtocol)
{
    if (message->isPacket())
        emit(packetSentToUpperSignal, message);
    send(message, "upperLayerOut", protocolMapping.getOutputGateForProtocol(transportProtocol));
}

void NetworkProtocolBase::sendDown(cMessage *message, int interfaceId)
{
    if (message->isPacket())
        emit(packetSentToLowerSignal, message);
    if (interfaceId != -1) {
        InterfaceEntry *interfaceEntry = interfaceTable->getInterfaceById(interfaceId);
        send(message, "lowerLayerOut", interfaceEntry->getNetworkLayerGateIndex());
    }
    else {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            InterfaceEntry *interfaceEntry = interfaceTable->getInterface(i);
            if (interfaceEntry && !interfaceEntry->isLoopback()) {
                cMessage *duplicate = message->dup();
                duplicate->setControlInfo(message->getControlInfo()->dup());
                send(duplicate, "lowerLayerOut", interfaceEntry->getNetworkLayerGateIndex());
            }
        }
        delete message;
    }
}

bool NetworkProtocolBase::isUpperMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("upperLayerIn");
}

bool NetworkProtocolBase::isLowerMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("lowerLayerIn");
}

} // namespace inet

