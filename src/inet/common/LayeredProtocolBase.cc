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

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/Simsignals.h"

namespace inet {

void LayeredProtocolBase::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else if (isUpperMessage(message))
        handleUpperMessage(message);
    else if (isLowerMessage(message))
        handleLowerMessage(message);
    else
        throw cRuntimeError("Message '%s' received on unexpected gate '%s'", message->getName(), message->getArrivalGate()->getFullName());
}

void LayeredProtocolBase::handleUpperMessage(cMessage *message)
{
    if (!message->isPacket())
        handleUpperCommand(message);
    else {
        emit(packetReceivedFromUpperSignal, message);
        handleUpperPacket(check_and_cast<Packet *>(message));
    }
}

void LayeredProtocolBase::handleLowerMessage(cMessage *message)
{
    if (!message->isPacket())
        handleLowerCommand(message);
    else {
        emit(packetReceivedFromLowerSignal, message);
        handleLowerPacket(check_and_cast<Packet *>(message));
    }
}

void LayeredProtocolBase::handleSelfMessage(cMessage *message)
{
    throw cRuntimeError("Self message '%s' is not handled.", message->getName());
}

void LayeredProtocolBase::handleUpperCommand(cMessage *message)
{
    throw cRuntimeError("Upper command '%s' is not handled.", message->getName());
}

void LayeredProtocolBase::handleLowerCommand(cMessage *message)
{
    throw cRuntimeError("Lower command '%s' is not handled.", message->getName());
}

void LayeredProtocolBase::handleUpperPacket(Packet *packet)
{
    throw cRuntimeError("Upper packet '%s' is not handled.", packet->getName());
}

void LayeredProtocolBase::handleLowerPacket(Packet *packet)
{
    throw cRuntimeError("Lower packet '%s' is not handled.", packet->getName());
}

} // namespace inet

