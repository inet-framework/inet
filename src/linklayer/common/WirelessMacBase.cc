/***************************************************************************
 * file:        WirelessMacBase.cc
 *
 * derived by Andras Varga using decremental programming from BasicMacLayer.cc,
 * which had the following copyright:
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU Lesser General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#include "WirelessMacBase.h"
#include "InterfaceEntry.h"
#include "NodeOperations.h"


simsignal_t WirelessMacBase::packetSentToLowerSignal = registerSignal("packetSentToLower");
simsignal_t WirelessMacBase::packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
simsignal_t WirelessMacBase::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t WirelessMacBase::packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");


void WirelessMacBase::initialize(int stage)
{
    MACBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        lowerLayerIn = findGate("lowerLayerIn");
        lowerLayerOut = findGate("lowerLayerOut");
    }
}

void WirelessMacBase::handleMessage(cMessage *msg)
{
    if (!isOperational)
    {
        handleMessageWhenDown(msg);
        return;
    }

    if (msg->isSelfMessage())
        handleSelfMsg(msg);
    else if (msg->getArrivalGateId()==upperLayerIn)
    {
        if (!msg->isPacket())
            handleCommand(msg);
        else
        {
            emit(packetReceivedFromUpperSignal, msg);
            handleUpperMsg(PK(msg));
        }
    }
    else if (msg->getArrivalGateId()==lowerLayerIn)
    {
        emit(packetReceivedFromLowerSignal, msg);
        handleLowerMsg(PK(msg));
    }
    else
        throw cRuntimeError("Message '%s' received on unexpected gate '%s'", msg->getName(), msg->getArrivalGate()->getFullName());
}

bool WirelessMacBase::isUpperMsg(cMessage *msg)
{
    return msg->getArrivalGateId()==upperLayerIn;
}

bool WirelessMacBase::isLowerMsg(cMessage *msg)
{
    return msg->getArrivalGateId()==lowerLayerIn;
}

void WirelessMacBase::sendDown(cMessage *msg)
{
    EV << "sending down " << msg << "\n";

    if (msg->isPacket())
        emit(packetSentToLowerSignal, msg);

    send(msg, lowerLayerOut);
}

void WirelessMacBase::sendUp(cMessage *msg)
{
    EV << "sending up " << msg << "\n";

    if (msg->isPacket())
        emit(packetSentToUpperSignal, msg);

    send(msg, upperLayerOut);
}

