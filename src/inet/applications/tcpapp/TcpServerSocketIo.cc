//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpServerSocketIo.h"

#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpServerSocketIo);

void TcpServerSocketIo::acceptSocket(TcpAvailableInfo *availableInfo)
{
    Enter_Method("acceptSocket");
    socket = new TcpSocket(availableInfo);
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);
    socket->accept(availableInfo->getNewSocketId());
}

void TcpServerSocketIo::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(socket != nullptr && socket->belongsToSocket(message));
        socket->processMessage(message);
    }
    else if (message->arrivedOn("trafficIn"))
        socket->send(check_and_cast<Packet *>(message));
    else
        throw cRuntimeError("Unknown message");
}

void TcpServerSocketIo::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    packet->removeTag<SocketInd>();
    send(packet, "trafficOut");
}

} // namespace inet

