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

TcpClientSocketIo::~TcpClientSocketIo()
{
    cancelAndDelete(readDelayTimer);
}

void TcpClientSocketIo::open()
{
    socket.setOutputGate(gate("socketOut"));
    socket.setCallback(this);
    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");
    bool autoRead = par("autoRead");
    L3Address destination = L3AddressResolver().resolve(connectAddress);
    socket.setAutoRead(autoRead);
    socket.connect(destination, connectPort);
    if (!autoRead)
        readDelayTimer = new cMessage("readDelayTimer");
}

void TcpClientSocketIo::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(socket.belongsToSocket(message));
        socket.processMessage(message);
    }
    else if (message->arrivedOn("trafficIn")) {
        if (!socket.isOpen())
            open();
        socket.send(check_and_cast<Packet *>(message));
    }
    else if (message == readDelayTimer) {
        socket.read(par("readSize"));
    }
    else
        throw cRuntimeError("Unknown message");
}

void TcpClientSocketIo::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    packet->removeTag<SocketInd>();
    send(packet, "trafficOut");
    sendOrScheduleReadCommandIfNeeded();
}

void TcpClientSocketIo::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo)
{
}

void TcpClientSocketIo::socketEstablished(TcpSocket *socket)
{
    sendOrScheduleReadCommandIfNeeded();
}

void TcpClientSocketIo::socketPeerClosed(TcpSocket *socket)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
}

void TcpClientSocketIo::socketClosed(TcpSocket *socket)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
}

void TcpClientSocketIo::socketFailure(TcpSocket *socket, int code)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
}

void TcpClientSocketIo::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status)
{
}

void TcpClientSocketIo::socketDeleted(TcpSocket *socket)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
}

void TcpClientSocketIo::sendOrScheduleReadCommandIfNeeded()
{
    if (!socket.getAutoRead() && socket.isOpen()) {
        simtime_t delay = par("readDelay");
        if (delay >= SIMTIME_ZERO)
            scheduleAfter(delay, readDelayTimer);
        else
            // send read message to TCP
            socket.read(par("readSize"));
    }
}

} // namespace inet
