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

namespace inet {

simsignal_t LayeredProtocolBase::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t LayeredProtocolBase::packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");
simsignal_t LayeredProtocolBase::packetFromUpperDroppedSignal = registerSignal("packetFromUpperDropped");

simsignal_t LayeredProtocolBase::packetSentToLowerSignal = registerSignal("packetSentToLower");
simsignal_t LayeredProtocolBase::packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
simsignal_t LayeredProtocolBase::packetFromLowerDroppedSignal = registerSignal("packetFromLowerDropped");

void LayeredProtocolBase::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else if (isUpperMessage(message)) {
        if (!message->isPacket())
            handleUpperCommand(message);
        else {
            emit(packetReceivedFromUpperSignal, message);
            handleUpperPacket(PK(message));
        }
    }
    else if (isLowerMessage(message)) {
        if (!message->isPacket())
            handleLowerCommand(message);
        else {
            emit(packetReceivedFromLowerSignal, message);
            handleLowerPacket(PK(message));
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

