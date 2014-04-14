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

#include "INETDefs.h"

#include "ByteArrayMessage.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "SocketsRTScheduler.h"
#include "TCPSrvHostApp.h"
#include "TCPTunnelThread.h"

class INET_API TCPClientTunnelThread : public TCPTunnelThread
{
  protected:
    int connSocket;

  public:
    TCPClientTunnelThread();
    virtual ~TCPClientTunnelThread();

    virtual void established();
    virtual void dataArrived(cMessage *msg, bool urgent);
    virtual void timerExpired(cMessage *timer); // for processing messages from SocketsRTScheduler

    /*
     * Called when the client closes the connection. By default it closes
     * our side too, but it can be redefined to do something different.
     */
    virtual void peerClosed();

    /*
     * Called when the connection closes (successful TCP teardown). By default
     * it deletes this thread, but it can be redefined to do something different.
     */
    virtual void closed();

    /*
     * Called when the connection breaks (TCP error). By default it deletes
     * this thread, but it can be redefined to do something different.
     */
    virtual void failure(int code);
};

class INET_API TCPClientTunnel : public TCPMultiThreadApp
{
  protected:
    SocketsRTScheduler *rtScheduler;
    int listenerSocket;
    uint16_t tunnelPort;

  public:
    TCPClientTunnel();
    virtual ~TCPClientTunnel();
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
};

Register_Class(TCPClientTunnelThread);
Register_Class(TCPClientTunnel);

TCPClientTunnelThread::TCPClientTunnelThread() :
        connSocket(INVALID_SOCKET)
{
}

TCPClientTunnelThread::~TCPClientTunnelThread()
{
    if (connSocket != INVALID_SOCKET)
    {
        closesocket(connSocket);
        check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    }
}

void TCPClientTunnelThread::peerClosed()
{
    EV_TRACE << "peerClosed()";
    close(connSocket);
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    connSocket = INVALID_SOCKET;
    TCPTunnelThread::peerClosed();
}

void TCPClientTunnelThread::closed()
{
    EV_TRACE << "closed()";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    connSocket = INVALID_SOCKET;
    TCPTunnelThread::closed();
}

void TCPClientTunnelThread::failure(int code)
{
    EV_TRACE << "failure()";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    TCPTunnelThread::failure(code);
}

void TCPClientTunnelThread::established()
{
    EV_TRACE << "established()";

    int err;

    if (connSocket != INVALID_SOCKET)
        throw cRuntimeError("model error: socket already opened");;

    // Create a TCP socket:
    connSocket = ::socket(AF_INET, SOCK_STREAM, 0);

    // Use bind to set an address and port number for our end of the finger TCP connection:
    sockaddr_in sinInterface;
    bzero(&sinInterface, sizeof(sinInterface));
    sinInterface.sin_family = AF_INET;
    sinInterface.sin_port = htons(0); /* tells OS to choose a port */
    sinInterface.sin_addr.s_addr = htonl(INADDR_ANY); /* tells OS to choose IP addr */
    if (::bind(connSocket, (struct sockaddr *)&sinInterface, sizeof(sinInterface)) < 0)
    {
        closesocket(connSocket);
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
    err = ::connect(connSocket, (struct sockaddr *)&sinInterface, sizeof(sinInterface));
    if (err < 0)
    {
        closesocket(connSocket);
        throw cRuntimeError("connect error to %s:%d : %d", tunnelAddr.str().c_str(), tunnelPort, err);
    }
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->addSocket(hostmod, this, connSocket, false);
}

void TCPClientTunnelThread::dataArrived(cMessage *msg, bool urgent)
{
    EV_TRACE << "dataArrived(" << msg << ", " << urgent << ")";
    ByteArray& bytes = check_and_cast<ByteArrayMessage *>(msg)->getByteArray();
    ::send(connSocket, bytes.getDataArrayPointer(), bytes.getDataArraySize(), 0);
    delete msg;
}

void TCPClientTunnelThread::timerExpired(cMessage *msg) // for processing messages from SocketsRTScheduler
{
    EV_TRACE << "timerExpired(" << msg <<  ")";
    switch(msg->getKind())
    {
        case SocketsRTScheduler::DATA:
        {
            if (connSocket != msg->par("fd").longValue())
                throw cRuntimeError("socket not opened");
            ByteArrayMessage *pk = check_and_cast<ByteArrayMessage *>(msg);
            sock->send(pk);
            break;
        }
        case SocketsRTScheduler::CLOSED:
            if (connSocket != msg->par("fd").longValue())
                throw cRuntimeError("unknown socket id");
            sock->close();
            delete msg;
            break;
        case SocketsRTScheduler::ACCEPT:
            throw cRuntimeError("Model error: nem hasznalunk listener portot"); //FIXME translate
        default:
            throw cRuntimeError("Invalid msg kind: %d", msg->getKind());
    }
}


TCPClientTunnel::TCPClientTunnel() :
        rtScheduler(NULL), listenerSocket(INVALID_SOCKET)
{
}

TCPClientTunnel::~TCPClientTunnel()
{
    closesocket(listenerSocket);
    rtScheduler->removeAllSocketOf(this);
}

void TCPClientTunnel::initialize(int stage)
{
    TCPMultiThreadApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        rtScheduler = check_and_cast<SocketsRTScheduler *>(simulation.getScheduler());
    }
}

