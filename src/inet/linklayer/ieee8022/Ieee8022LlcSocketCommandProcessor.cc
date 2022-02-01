//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommandProcessor.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"

namespace inet {

Define_Module(Ieee8022LlcSocketCommandProcessor);

void Ieee8022LlcSocketCommandProcessor::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        socketTable.reference(this, "socketTableModule", true);
}

cGate *Ieee8022LlcSocketCommandProcessor::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void Ieee8022LlcSocketCommandProcessor::handleMessage(cMessage *message)
{
    if (auto request = dynamic_cast<Request *>(message))
        handleCommand(request);
    else
        PacketFlowBase::handleMessage(message);
}

void Ieee8022LlcSocketCommandProcessor::handleCommand(Request *request)
{
    auto controlInfo = request->getControlInfo();
    if (auto command = dynamic_cast<Ieee8022LlcSocketOpenCommand *>(controlInfo)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->addSocket(socketId, command->getLocalSap(), -1);
        delete request;
    }
    else if (dynamic_cast<SocketCloseCommand *>(controlInfo) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->removeSocket(socketId);
        delete request;
        // TODO move to Ieee8022SocketPacketProcessor module into a listener on the Ieee8022SocketTable
//        auto indication = new Indication("closed", IEEE8022_LLC_I_SOCKET_CLOSED);
//        auto controlInfo = new Ieee8022LlcSocketClosedIndication();
//        indication->setControlInfo(controlInfo);
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

