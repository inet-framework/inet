//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetSocketCommandProcessor.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetCommand_m.h"

namespace inet {

Define_Module(EthernetSocketCommandProcessor);

void EthernetSocketCommandProcessor::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        socketTable.reference(this, "socketTableModule", true);
}

cGate *EthernetSocketCommandProcessor::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void EthernetSocketCommandProcessor::handleMessage(cMessage *message)
{
    if (auto request = dynamic_cast<Request *>(message))
        handleCommand(request);
    else
        PacketFlowBase::handleMessage(message);
}

void EthernetSocketCommandProcessor::handleCommand(Request *request)
{
    auto controlInfo = request->getControlInfo();
    if (auto bindCommand = dynamic_cast<EthernetBindCommand *>(controlInfo)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->addSocket(socketId, bindCommand->getLocalAddress(), bindCommand->getRemoteAddress(), bindCommand->getProtocol(), bindCommand->getSteal());
        delete request;
    }
    else if (dynamic_cast<SocketCloseCommand *>(controlInfo) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->removeSocket(socketId);
        delete request;
        // TODO move to EthernetSocketPacketProcessor module into a listener on the EthernetSocketTable
//        auto indication = new Indication("closed", ETHERNET_I_SOCKET_CLOSED);
//        auto ctrl = new EthernetSocketClosedIndication();
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

