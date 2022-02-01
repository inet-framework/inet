//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qSocketCommandProcessor.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qCommand_m.h"

namespace inet {

Define_Module(Ieee8021qSocketCommandProcessor);

void Ieee8021qSocketCommandProcessor::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        socketTable.reference(this, "socketTableModule", true);
}

cGate *Ieee8021qSocketCommandProcessor::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void Ieee8021qSocketCommandProcessor::handleMessage(cMessage *message)
{
    if (auto request = dynamic_cast<Request *>(message))
        handleCommand(request);
    else
        PacketFlowBase::handleMessage(message);
}

void Ieee8021qSocketCommandProcessor::handleCommand(Request *request)
{
    auto controlInfo = request->getControlInfo();
    if (auto bindCommand = dynamic_cast<Ieee8021qBindCommand *>(controlInfo)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->addSocket(socketId, bindCommand->getProtocol(), bindCommand->getVlanId(), bindCommand->getSteal());
        delete request;
    }
    else if (dynamic_cast<SocketCloseCommand *>(controlInfo) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->removeSocket(socketId);
        delete request;
        // TODO move to Ieee8021qSocketPacketProcessor module into a listener on the Ieee8021qSocketTable
//        auto indication = new Indication("closed", ETHERNET_I_SOCKET_CLOSED);
//        auto ctrl = new Ieee8021qSocketClosedIndication();
//        indication->setControlInfo(ctrl);
//        indication->addTag<SocketInd>()->setSocketId(socketId);
//        send(indication, "cmdOut");
    }
    else if (dynamic_cast<SocketDestroyCommand *>(controlInfo) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->removeSocket(socketId);
        delete request;
    }
    else
        throw cRuntimeError("Unknown request");
}

} // namespace inet

