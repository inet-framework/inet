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
#include "NotificationBoard.h"
#include "InterfaceEntry.h"
#include "NodeOperations.h"


simsignal_t WirelessMacBase::packetSentToLowerSignal = SIMSIGNAL_NULL;
simsignal_t WirelessMacBase::packetReceivedFromLowerSignal = SIMSIGNAL_NULL;
simsignal_t WirelessMacBase::packetSentToUpperSignal = SIMSIGNAL_NULL;
simsignal_t WirelessMacBase::packetReceivedFromUpperSignal = SIMSIGNAL_NULL;

void WirelessMacBase::initialize(int stage)
{
    if (stage==0)
    {
        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        lowerLayerIn = findGate("lowerLayerIn");
        lowerLayerOut = findGate("lowerLayerOut");

        // get a pointer to the NotificationBoard module
        nb = NotificationBoardAccess().get();
        cModule * node = findContainingNode(this);
        nodeStatus = dynamic_cast<NodeStatus *>(node->getSubmodule("status"));

        packetSentToLowerSignal = registerSignal("packetSentToLower");
        packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
        packetSentToUpperSignal = registerSignal("packetSentToUpper");
        packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");
    }
}

bool WirelessMacBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_LINK_LAYER)
            ; // TODO: registerInterface();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        // TODO:
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        // TODO:
    }
    else if (dynamic_cast<InterfaceUpOperation *>(operation)) {
        // TODO:
    }
    else if (dynamic_cast<InterfaceDownOperation *>(operation)) {
        // TODO:
    }
    return true;
}

void WirelessMacBase::handleMessage(cMessage *msg)
{
    bool isNodeUp = !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
    bool isInterfaceUp = !interfaceEntry || interfaceEntry->getState() == InterfaceEntry::UP;
    if (!isNodeUp || !isInterfaceUp)
    {
        if (!msg->arrivedOn("phys$i") || msg->isSelfMessage())  //FIXME remove 1st part -- it is not possible to ensure that no msg is sent by upper layer (race condition!!!)
            throw cRuntimeError("Interface is turned off");
        else {
            EV << "Interface is turned off, dropping packet\n";
            delete msg;
            return;
        }
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

