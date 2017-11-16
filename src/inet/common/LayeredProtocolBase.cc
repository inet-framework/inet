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
    else if (isUpperMessage(message)) {
        if (!message->isPacket())
            handleUpperCommand(message);
        else {
            emit(packetReceivedFromUpperSignal, message);
            handleUpperPacket(check_and_cast<Packet *>(message));
        }
    }
    else if (isLowerMessage(message)) {
        if (!message->isPacket())
            handleLowerCommand(message);
        else {
            emit(packetReceivedFromLowerSignal, message);
            handleLowerPacket(check_and_cast<Packet *>(message));
        }
    }
    else
        throw cRuntimeError("Message '%s' received on unexpected gate '%s'", message->getName(), message->getArrivalGate()->getFullName());
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

} // namespace inet

