//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// author: Zoltan Bojthe

#include <platdep/sockets.h>

#include  "TCPTunnelThreadBase.h"

#include "ByteArrayMessage.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "SocketsRTScheduler.h"
#include "TCPSrvHostApp.h"


TCPTunnelThreadBase::TCPTunnelThreadBase()
  : realSocket(INVALID_SOCKET)
{
}

TCPTunnelThreadBase::~TCPTunnelThreadBase()
{
    if (realSocket != INVALID_SOCKET)
    {
        closesocket(realSocket);
        check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(this, realSocket);
    }
}

void TCPTunnelThreadBase::initialize(int stage)
{
    TCPThreadBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        WATCH(realSocket);
    }
}

void TCPTunnelThreadBase::socketPeerClosed(int connId, void *yourPtr)
{
    EV_TRACE << "peerClosed()\n";
    close(realSocket);
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(this, realSocket);
    realSocket = INVALID_SOCKET;
    TCPThreadBase::socketPeerClosed(connId, yourPtr);
}

void TCPTunnelThreadBase::socketClosed(int connId, void *yourPtr)
{
    EV_TRACE << "closed()\n";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(this, realSocket);
    realSocket = INVALID_SOCKET;
    TCPThreadBase::socketClosed(connId, yourPtr);
}

void TCPTunnelThreadBase::socketFailure(int connId, void *yourPtr, int code)
{
    EV_TRACE << "failure(" << code << ")\n";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(this, realSocket);
    TCPThreadBase::socketFailure(connId, yourPtr, code);
}

void TCPTunnelThreadBase::socketEstablished(int connId, void *yourPtr)
{
    EV_TRACE << "established()\n";

    int err;

    if (realSocket != INVALID_SOCKET)
        throw cRuntimeError("model error: socket already opened");;

    // Create a TCP socket:
    realSocket = ::socket(AF_INET, SOCK_STREAM, 0);

    // Use bind to set an address and port number for our end of the finger TCP connection:
    sockaddr_in sinInterface;
    bzero(&sinInterface, sizeof(sinInterface));
    sinInterface.sin_family = AF_INET;
    sinInterface.sin_port = htons(0); /* tells OS to choose a port */
    sinInterface.sin_addr.s_addr = htonl(INADDR_ANY); /* tells OS to choose IP addr */
    if (::bind(realSocket, (struct sockaddr *)&sinInterface, sizeof(sinInterface)) < 0)
    {
        closesocket(realSocket);
        throw cRuntimeError("bind error");
    }
    // Set the destination address:
    IPv4Address tunnelAddr(hostmod->par("tunnelAddress").stringValue());
    int tunnelPort = hostmod->par("tunnelPort");
    bzero(&sinInterface, sizeof(sinInterface));
    sinInterface.sin_family = AF_INET;
    sinInterface.sin_port = htons(tunnelPort);
    sinInterface.sin_addr.s_addr = htonl(tunnelAddr.getInt());
    // Connect to the server
    err = ::connect(realSocket, (struct sockaddr *)&sinInterface, sizeof(sinInterface));
    if (err < 0)
    {
        closesocket(realSocket);
        throw cRuntimeError("connect error to %s:%d : %d", tunnelAddr.str().c_str(), tunnelPort, err);
    }
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->addSocket(hostmod, this, realSocket, false);
}

void TCPTunnelThreadBase::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
    EV_TRACE << "dataArrived(" << msg << ", " << urgent << ")\n";
    ByteArray& bytes = check_and_cast<ByteArrayMessage *>(msg)->getByteArray();
    ::send(realSocket, bytes.getDataArrayPointer(), bytes.getDataArraySize(), 0);
    delete msg;
}

void TCPTunnelThreadBase::handleSelfMessage(cMessage *msg) // for processing messages from SocketsRTScheduler
{
    EV_TRACE << "timerExpired(" << msg <<  ")\n";
    switch(msg->getKind())
    {
        case SocketsRTScheduler::DATA:
            realDataArrived(check_and_cast<ByteArrayMessage *>(msg));
            break;
        case SocketsRTScheduler::CLOSED:
            realSocketClosed(msg);
            break;
        case SocketsRTScheduler::ACCEPT:
            realSocketAccept(msg);
            break;
        default:
            throw cRuntimeError("Invalid message kind %d arrived from SocketsRTScheduler", msg->getKind());
    }
}

void TCPTunnelThreadBase::realDataArrived(ByteArrayMessage *msg)
{
    EV_TRACE << "realDataArrived(" << msg << ")\n";
    int fd = msg->par("fd").longValue();
    if (realSocket != fd)
        throw cRuntimeError("Model error: socket problem: realSocket=%d, fd=%d", realSocket, fd);
    socket.send(msg);
}

void TCPTunnelThreadBase::realSocketClosed(cMessage *msg)
{
    EV_TRACE << "realSocketClosed(" << msg << ")\n";
    if (realSocket != msg->par("fd").longValue())
        throw cRuntimeError("unknown socket id");
    socket.close();
    delete msg;
}

void TCPTunnelThreadBase::realSocketAccept(cMessage *msg)
{
    EV_TRACE << "realSocketAccept(" << msg << ")\n";
    throw cRuntimeError("Model error: this thread not a listener thread\n");
}

