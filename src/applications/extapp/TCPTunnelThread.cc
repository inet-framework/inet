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

#include  "TCPTunnelThread.h"

#include "ByteArrayMessage.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "SocketsRTScheduler.h"
#include "TCPSrvHostApp.h"

Register_Class(TCPTunnelThread);

TCPTunnelThread::TCPTunnelThread() :
        realSocket(INVALID_SOCKET)
{
}

TCPTunnelThread::~TCPTunnelThread()
{
    if (realSocket != INVALID_SOCKET)
    {
        closesocket(realSocket);
        check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, realSocket);
    }
}

void TCPTunnelThread::peerClosed()
{
    EV_TRACE << "peerClosed()";
    close(realSocket);
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, realSocket);
    realSocket = INVALID_SOCKET;
    TCPThreadBase::peerClosed();
}

void TCPTunnelThread::closed()
{
    EV_TRACE << "closed()";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, realSocket);
    realSocket = INVALID_SOCKET;
    TCPThreadBase::closed();
}

void TCPTunnelThread::failure(int code)
{
    EV_TRACE << "failure()";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, realSocket);
    TCPThreadBase::failure(code);
}

void TCPTunnelThread::established()
{
    EV_TRACE << "established()";

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

void TCPTunnelThread::dataArrived(cMessage *msg, bool urgent)
{
    EV_TRACE << "dataArrived(" << msg << ", " << urgent << ")";
    ByteArray& bytes = check_and_cast<ByteArrayMessage *>(msg)->getByteArray();
    ::send(realSocket, bytes.getDataArrayPointer(), bytes.getDataArraySize(), 0);
    delete msg;
}

void TCPTunnelThread::timerExpired(cMessage *msg) // for processing messages from SocketsRTScheduler
{
    EV_TRACE << "timerExpired(" << msg <<  ")";
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

void TCPTunnelThread::realDataArrived(ByteArrayMessage *msg)
{
    if (realSocket != msg->par("fd").longValue())
        throw cRuntimeError("socket not opened");
    sock->send(msg);
}

void TCPTunnelThread::realSocketClosed(cMessage *msg)
{
    if (realSocket != msg->par("fd").longValue())
        throw cRuntimeError("unknown socket id");
    sock->close();
    delete msg;
}

void TCPTunnelThread::realSocketAccept(cMessage *msg)
{
    throw cRuntimeError("Model error: nem hasznalunk listener portot"); //FIXME translate
}

