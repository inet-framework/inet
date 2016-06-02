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
#include "inet/common/IInterfaceControlInfo.h"

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
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerProtocol(Protocol::gnp, gate("transportOut"));
        registerProtocol(Protocol::gnp, gate("queueOut"));
    }
}

void NetworkProtocolBase::handleRegisterProtocol(const Protocol& protocol, cGate *gate) {
    Enter_Method("handleRegisterProtocol");
    protocolMapping.addProtocolMapping(protocol.getId(), gate->getIndex());
}

void NetworkProtocolBase::sendUp(cMessage *message)
{
    if (message->isPacket())
        emit(packetSentToUpperSignal, message);
    send(message, "transportOut");
}

void NetworkProtocolBase::sendDown(cMessage *message, int interfaceId)
{
    if (message->isPacket())
        emit(packetSentToLowerSignal, message);
    if (interfaceId != -1) {
        cObject *ctrl = message->getControlInfo();
        check_and_cast<IInterfaceControlInfo *>(ctrl)->setInterfaceId(interfaceId);
        send(message, "queueOut");
    }
    else {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            InterfaceEntry *interfaceEntry = interfaceTable->getInterface(i);
            if (interfaceEntry && !interfaceEntry->isLoopback()) {
                cMessage *duplicate = message->dup();
                cObject *ctrl = message->getControlInfo()->dup();
                check_and_cast<IInterfaceControlInfo *>(ctrl)->setInterfaceId(interfaceEntry->getInterfaceId());
                duplicate->setControlInfo(ctrl);
                send(duplicate, "queueOut");
            }
        }
        delete message;
    }
}

bool NetworkProtocolBase::isUpperMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("transportIn");
}

bool NetworkProtocolBase::isLowerMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("queueIn");
}

} // namespace inet

