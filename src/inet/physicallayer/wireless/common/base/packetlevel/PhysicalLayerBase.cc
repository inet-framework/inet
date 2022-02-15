//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/PhysicalLayerBase.h"

#include "inet/common/Simsignals.h"

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
        handleSignal(check_and_cast<WirelessSignal *>(message));
    }
}

void PhysicalLayerBase::handleSignal(WirelessSignal *signal)
{
    throw cRuntimeError("Signal '%s' is not handled.", signal->getName());
}

void PhysicalLayerBase::sendUp(cMessage *message)
{
    if (message->isPacket())
        emit(packetSentToUpperSignal, message);
    send(message, upperLayerOutGateId);
}

bool PhysicalLayerBase::isUpperMessage(cMessage *message) const
{
    return message->getArrivalGateId() == upperLayerInGateId;
}

bool PhysicalLayerBase::isLowerMessage(cMessage *message) const
{
    return message->getArrivalGateId() == radioInGateId;
}

} // namespace physicallayer

} // namespace inet

