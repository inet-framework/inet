//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4SocketCommandProcessor.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4SocketCommand_m.h"

namespace inet {

Define_Module(Ipv4SocketCommandProcessor);

void Ipv4SocketCommandProcessor::initialize(int stage)
{
    OperationalMixin<queueing::PacketFlowBase>::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        socketTable.reference(this, "socketTableModule", true);
        socketOutGate = gate("socketOut");
        socketPacketProcessorModule.reference(socketOutGate, false);
    }
}

cGate *Ipv4SocketCommandProcessor::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void Ipv4SocketCommandProcessor::handleMessageWhenUp(cMessage *message)
{
    if (auto command = dynamic_cast<Message *>(message))
        processPushCommand(command, command->getArrivalGate());
    else
        PacketFlowBase::handleMessage(message);
}

void Ipv4SocketCommandProcessor::processPushCommand(Message *command, cGate *arrivalGate)
{
    Request *request = check_and_cast<Request *>(command);
    auto ctrl = request->getControlInfo();
    if (ctrl == nullptr)
        throw cRuntimeError("Request '%s' arrived without controlinfo", request->getName());
    else if (Ipv4SocketBindCommand *command = dynamic_cast<Ipv4SocketBindCommand *>(ctrl)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->addSocket(socketId, command->getProtocol()->getId(), command->getLocalAddress());
        delete request;
    }
    else if (Ipv4SocketConnectCommand *command = dynamic_cast<Ipv4SocketConnectCommand *>(ctrl)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        if (!socketTable->connectSocket(socketId, command->getRemoteAddress()))
            throw cRuntimeError("Ipv4Socket: should use bind() before connect()");
        delete request;
    }
    else if (dynamic_cast<Ipv4SocketCloseCommand *>(ctrl) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        if (socketTable->removeSocket(socketId)) {
            auto indication = new Indication("closed", IPv4_I_SOCKET_CLOSED);
            auto ctrl = new Ipv4SocketClosedIndication();
            indication->setControlInfo(ctrl);
            indication->addTag<SocketInd>()->setSocketId(socketId);
            pushOrSendCommand(indication, socketOutGate, socketPacketProcessorModule);
        }
        delete request;
    }
    else if (dynamic_cast<Ipv4SocketDestroyCommand *>(ctrl) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->removeSocket(socketId);
        delete request;
    }
    else
        throw cRuntimeError("Unknown command: '%s' with %s", request->getName(), ctrl->getClassName());
}

void Ipv4SocketCommandProcessor::pushOrSendCommand(Message *command, cGate *outGate, Ipv4SocketPacketProcessor *consumer)
{
    if (consumer != nullptr) {
        //animatePushPacket(packet, gate);
        consumer->processPushCommand(command, findConnectedGate<Ipv4SocketPacketProcessor>(outGate));
    }
    else
        send(command, outGate);
}

} // namespace inet

