//
// Copyright (C) 2013 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/Simsignals.h"
#include "inet/physicallayer/base/packetlevel/PhysicalLayerBase.h"

namespace inet {

namespace physicallayer {

void PhysicalLayerBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        upperLayerInGateId = findGate("upperLayerIn");
        upperLayerOutGateId = findGate("upperLayerOut");
        radioInGateId = findGate("radioIn");
    }
}

void PhysicalLayerBase::handleLowerMessage(cMessage *message)
{
    if (!message->isPacket())
        handleLowerCommand(message);
    else {
        emit(packetReceivedFromLowerSignal, message);
        handleSignal(check_and_cast<Signal *>(message));
    }
}

void PhysicalLayerBase::handleSignal(Signal *signal)
{
    throw cRuntimeError("Signal '%s' is not handled.", signal->getName());
}

void PhysicalLayerBase::sendUp(cMessage *message)
{
    if (message->isPacket())
        emit(packetSentToUpperSignal, message);
    send(message, upperLayerOutGateId);
}

bool PhysicalLayerBase::isUpperMessage(cMessage *message)
{
    return message->getArrivalGateId() == upperLayerInGateId;
}

bool PhysicalLayerBase::isLowerMessage(cMessage *message)
{
    return message->getArrivalGateId() == radioInGateId;
}

} // namespace physicallayer

} // namespace inet

