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
#include "TCPAppBase.h"

class INET_API TCPClientTunnel : public TCPAppBase, public ILifecycle
{
  protected:
    SocketsRTScheduler *rtScheduler;
    int listenerSocket;
    int connSocket;

  public:
    TCPClientTunnel();
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleTimer(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketPeerClosed(int connId, void *yourPtr);
    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) { delete status; }
};

Register_Class(TCPClientTunnel);

TCPClientTunnel::TCPClientTunnel()
    : rtScheduler(NULL), listenerSocket(INVALID_SOCKET), connSocket(INVALID_SOCKET)
{
}

void TCPClientTunnel::initialize(int stage)
{
    TCPAppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        rtScheduler = check_and_cast<SocketsRTScheduler *>(simulation.getScheduler());
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(getContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        listenerSocket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listenerSocket == INVALID_SOCKET)
            throw cRuntimeError("cannot create socket");
        connSocket = INVALID_SOCKET;

        sockaddr_in sinInterface;
        sinInterface.sin_family = AF_INET;
        sinInterface.sin_addr.s_addr = INADDR_ANY;
        sinInterface.sin_port = htons(uint16_t(par("tunnelPort")));
        if (bind(listenerSocket, (sockaddr*)&sinInterface, sizeof(sockaddr_in)) == SOCKET_ERROR)
            throw cRuntimeError("socket bind() failed");

        listen(listenerSocket, SOMAXCONN);

        rtScheduler->addSocket(this, listenerSocket, true);
    }
}


void TCPClientTunnel::handleTimer(cMessage *msg)
{
    switch(msg->getKind())
    {
        case SocketsRTScheduler::DATA:
        {
            if (connSocket != msg->par("fd").longValue())
                throw cRuntimeError("socket not opened");
            ByteArrayMessage *pk = check_and_cast<ByteArrayMessage *>(msg);
            sendPacket(pk);
            break;
        }
        case SocketsRTScheduler::CLOSED:
            if (connSocket != msg->par("fd").longValue())
                throw cRuntimeError("unknown socket id");
            close();
            connSocket = INVALID_SOCKET;
            delete msg;
            break;
        case SocketsRTScheduler::ACCEPT:
            if (connSocket != INVALID_SOCKET)
                throw cRuntimeError("socket already opened");
            connSocket = msg->par("fd").longValue();
            rtScheduler->addSocket(this, connSocket, false);
            connect();
            delete msg;
            break;
    }
}

bool TCPClientTunnel::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
}

void TCPClientTunnel::socketDataArrived(int connId, void *ptr, cPacket *msg, bool urgent)
{
    ByteArray& bytes = check_and_cast<ByteArrayMessage *>(msg)->getByteArray();
    ::send(connSocket,bytes.getDataArrayPointer(), bytes.getDataArraySize(), 0);

    TCPAppBase::socketDataArrived(connId, ptr, msg, urgent);
}

void TCPClientTunnel::socketEstablished(int connId, void *ptr)
{
    TCPAppBase::socketEstablished(connId, ptr);
}

void TCPClientTunnel::socketPeerClosed(int connId, void *ptr)
{
    TCPAppBase::socketPeerClosed(connId, ptr);
    if (connSocket != INVALID_SOCKET)
    {
        closesocket(connSocket);
        rtScheduler->removeSocket(this, connSocket);
        connSocket = INVALID_SOCKET;
    }
}

void TCPClientTunnel::socketClosed(int connId, void *ptr)
{
    TCPAppBase::socketClosed(connId, ptr);
    if (connSocket != INVALID_SOCKET)
    {
        closesocket(connSocket);
        rtScheduler->removeSocket(this, connSocket);
        connSocket = INVALID_SOCKET;
    }
}

void TCPClientTunnel::socketFailure(int connId, void *ptr, int code)
{
    TCPAppBase::socketFailure(connId, ptr, code);
    if (connSocket != INVALID_SOCKET)
    {
        closesocket(connSocket);
        rtScheduler->removeSocket(this, connSocket);
        connSocket = INVALID_SOCKET;
    }
}

