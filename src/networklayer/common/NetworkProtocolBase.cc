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

#include "NetworkProtocolBase.h"
#include "InterfaceTableAccess.h"
#include "INetworkLayer.h"

NetworkProtocolBase::NetworkProtocolBase() :
    interfaceTable(NULL)
{
}

void NetworkProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        interfaceTable = InterfaceTableAccess().get(this);
    }
}

void NetworkProtocolBase::handleUpperCommand(cMessage* message)
{
    if (message->getKind() == MK_REGISTER_TRANSPORT_PROTOCOL)
    {
        RegisterTransportProtocolCommand * command = check_and_cast<RegisterTransportProtocolCommand *>(message->getControlInfo());
        protocolMapping.addProtocolMapping(command->getProtocol(), message->getArrivalGate()->getIndex());
        delete message;
    }
    else
        LayeredProtocolBase::handleUpperCommand(message);
}

void NetworkProtocolBase::sendUp(cMessage* message, int transportProtocol)
{
    if (message->isPacket())
        emit(packetSentToUpperSignal, message);
    send(message, "upperLayerOut", protocolMapping.getOutputGateForProtocol(transportProtocol));
}

void NetworkProtocolBase::sendDown(cMessage* message, int interfaceId)
{
    if (message->isPacket())
        emit(packetSentToLowerSignal, message);
    if (interfaceId != -1)
    {
        InterfaceEntry *interfaceEntry = interfaceTable->getInterfaceById(interfaceId);
        send(message, "lowerLayerOut", interfaceEntry->getNetworkLayerGateIndex());
    }
    else
    {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        {
            InterfaceEntry *interfaceEntry = interfaceTable->getInterface(i);
            if (interfaceEntry && !interfaceEntry->isLoopback())
            {
                cMessage* duplicate = message->dup();
                duplicate->setControlInfo(message->getControlInfo()->dup());
                send(duplicate, "lowerLayerOut", interfaceEntry->getNetworkLayerGateIndex());
            }
        }
        delete message;
    }
}

bool NetworkProtocolBase::isUpperMessage(cMessage* message)
{
    return message->getArrivalGate()->isName("upperLayerIn");
}

bool NetworkProtocolBase::isLowerMessage(cMessage* message)
{
    return message->getArrivalGate()->isName("lowerLayerIn");
}
