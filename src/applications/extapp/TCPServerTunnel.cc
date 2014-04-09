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

class INET_API TCPServerTunnelThread : public TCPServerThreadBase
{
  protected:
    int connSocket;

  public:
    TCPServerTunnelThread();
    virtual ~TCPServerTunnelThread();

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

class INET_API TCPServerTunnel : public TCPSrvHostApp
{
  protected:
    SocketsRTScheduler *rtScheduler;

  public:
    TCPServerTunnel();
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
};

Register_Class(TCPServerTunnelThread);
Register_Class(TCPServerTunnel);

TCPServerTunnelThread::TCPServerTunnelThread() :
        connSocket(INVALID_SOCKET)
{
}

TCPServerTunnelThread::~TCPServerTunnelThread()
{
    if (connSocket != INVALID_SOCKET)
    {
        closesocket(connSocket);
        check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    }
}

void TCPServerTunnelThread::peerClosed()
{
    EV_TRACE << "peerClosed()";
    close(connSocket);
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    connSocket = INVALID_SOCKET;
    TCPServerThreadBase::peerClosed();
}

void TCPServerTunnelThread::closed()
{
    EV_TRACE << "closed()";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    connSocket = INVALID_SOCKET;
    TCPServerThreadBase::closed();
}

void TCPServerTunnelThread::failure(int code)
{
    EV_TRACE << "failure()";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    TCPServerThreadBase::failure(code);
}

void TCPServerTunnelThread::established()
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

void TCPServerTunnelThread::dataArrived(cMessage *msg, bool urgent)
{
    EV_TRACE << "dataArrived(" << msg << ", " << urgent << ")";
    ByteArray& bytes = check_and_cast<ByteArrayMessage *>(msg)->getByteArray();
    ::send(connSocket, bytes.getDataArrayPointer(), bytes.getDataArraySize(), 0);
    delete msg;
}

void TCPServerTunnelThread::timerExpired(cMessage *msg) // for processing messages from SocketsRTScheduler
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


TCPServerTunnel::TCPServerTunnel() :
        rtScheduler(NULL)
{
}

void TCPServerTunnel::initialize(int stage)
{
    TCPSrvHostApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        rtScheduler = check_and_cast<SocketsRTScheduler *>(simulation.getScheduler());
    }
}

