//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpClientSocketIo.h"

#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpClientSocketIo);

void TcpClientSocketIo::open()
{
    socket = new TcpSocket();
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);
    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    socket->bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");
    L3Address destination;
    L3AddressResolver().tryResolve(connectAddress, destination);
    socket->connect(destination, connectPort);
}

void TcpClientSocketIo::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(socket && socket->belongsToSocket(message));
        socket->processMessage(message);
    }
    else if (message->arrivedOn("trafficIn")) {
        if (socket == nullptr)
            open();
        socket->send(check_and_cast<Packet *>(message));
    }
    else
        throw cRuntimeError("Unknown message");
}

void TcpClientSocketIo::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    packet->removeTag<SocketInd>();
    send(packet, "trafficOut");
}

} // namespace inet

