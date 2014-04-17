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
#include "TCPMultiThreadApp.h"
#include "TCPTunnelThreadBase.h"


/**
 * listen on a real socket on localhost, create threads for accepted real connections
 */
class INET_API TCPClientTunnel : public TCPMultiThreadApp
{
  protected:
    SocketsRTScheduler *rtScheduler;

  public:
    TCPClientTunnel();
    virtual ~TCPClientTunnel();
    TCPThreadBase *createNewThreadFor(cMessage *msg);

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleSelfMessage(cMessage *msg) { throw cRuntimeError("this module doesn't use self messages"); };
};

//Register_Class(TCPClientTunnelThread);
Define_Module(TCPClientTunnel);

#if 0
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

void TCPClientTunnelThread::socketPeerClosed(int connId, void *yourPtr)
{
    EV_TRACE << "peerClosed()";
    close(connSocket);
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    connSocket = INVALID_SOCKET;
    TCPTunnelThreadBase::socketPeerClosed(connId, yourPtr);
}

void TCPClientTunnelThread::socketClosed(int connId, void *yourPtr)
{
    EV_TRACE << "closed()";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    connSocket = INVALID_SOCKET;
    TCPTunnelThreadBase::socketClosed(connId, yourPtr);
}

void TCPClientTunnelThread::socketFailure(int connId, void *yourPtr, int code)
{
    EV_TRACE << "failure()";
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->removeSocket(hostmod, connSocket);
    TCPTunnelThreadBase::socketFailure(connId, yourPtr, code);
}

void TCPClientTunnelThread::socketEstablished(int connId, void *yourPtr)
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

void TCPClientTunnelThread::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
    EV_TRACE << "dataArrived(" << msg << ", " << urgent << ")";
    ByteArray& bytes = check_and_cast<ByteArrayMessage *>(msg)->getByteArray();
    ::send(connSocket, bytes.getDataArrayPointer(), bytes.getDataArraySize(), 0);
    delete msg;
}

void TCPClientTunnelThread::handleSelfMessage(cMessage *msg) // for processing messages from SocketsRTScheduler
{
    EV_TRACE << "handleSelfMessage(" << msg <<  ")";
    switch(msg->getKind())
    {
        case SocketsRTScheduler::DATA:
        {
            if (connSocket != msg->par("fd").longValue())
                throw cRuntimeError("socket not opened");
            ByteArrayMessage *pk = check_and_cast<ByteArrayMessage *>(msg);
            socket.send(pk);
            break;
        }
        case SocketsRTScheduler::CLOSED:
            if (connSocket != msg->par("fd").longValue())
                throw cRuntimeError("unknown socket id");
            socket.close();
            delete msg;
            break;
        case SocketsRTScheduler::ACCEPT:
            throw cRuntimeError("Model error: nem hasznalunk listener portot"); //FIXME translate
        default:
            throw cRuntimeError("Invalid msg kind: %d", msg->getKind());
    }
}
#endif

TCPClientTunnel::TCPClientTunnel() :
        rtScheduler(NULL)
{
}

TCPClientTunnel::~TCPClientTunnel()
{
}

void TCPClientTunnel::initialize(int stage)
{
    TCPMultiThreadApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        rtScheduler = check_and_cast<SocketsRTScheduler *>(simulation.getScheduler());
    }
}

TCPThreadBase *TCPClientTunnel::createNewThreadFor(cMessage *msg)
{
    TCPTunnelThreadBase *thread = check_and_cast<TCPTunnelThreadBase *>(TCPMultiThreadApp::createNewThreadFor(msg));
    return thread;
}

