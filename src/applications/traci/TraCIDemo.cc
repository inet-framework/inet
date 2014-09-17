//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "inet/applications/traci/TraCIDemo.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

Define_Module(TraCIDemo);

simsignal_t TraCIDemo::mobilityStateChangedSignal = registerSignal("mobilityStateChanged");

void TraCIDemo::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        sentMessage = false;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        traci = getModuleFromPar<TraCIMobility>(par("mobilityModule"), this);
        traci->subscribe(mobilityStateChangedSignal, this);

        setupLowerLayer();
    }
}

void TraCIDemo::setupLowerLayer()
{
    socket.setOutputGate(gate("udp$o"));
    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    socket.joinLocalMulticastGroups(mgl);
    socket.bind(12345);
    socket.setBroadcast(true);
}

void TraCIDemo::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
    }
    else {
        handleLowerMsg(msg);
    }
}

void TraCIDemo::handleSelfMsg(cMessage *msg)
{
}

void TraCIDemo::handleLowerMsg(cMessage *msg)
{
    if (!sentMessage)
        sendMessage();
    delete msg;
}

void TraCIDemo::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate();
    }
}

void TraCIDemo::sendMessage()
{
    sentMessage = true;

    cPacket *newMessage = new cPacket();

    socket.sendTo(newMessage, IPv4Address::ALL_HOSTS_MCAST, 12345);
}

void TraCIDemo::handlePositionUpdate()
{
    if (traci->getPosition().x < 7350) {
        if (!sentMessage)
            sendMessage();
    }
}

} // namespace inet

