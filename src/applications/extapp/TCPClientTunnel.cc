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

#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "SocketsRTScheduler.h"

class TCPClientTunnel : public cSimpleModule, public ILifecycle
{
  protected:
    SocketsRTScheduler *rtScheduler;
    int listenerSocket;

  public:
    TCPClientTunnel();
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
};

TCPClientTunnel::TCPClientTunnel()
    : rtScheduler(NULL), listenerSocket(INVALID_SOCKET)
{
}

void TCPClientTunnel::initialize(int stage)
{
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

        listenerSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listenerSocket == INVALID_SOCKET)
            throw cRuntimeError("cannot create socket");

        sockaddr_in sinInterface;
        sinInterface.sin_family = AF_INET;
        sinInterface.sin_addr.s_addr = INADDR_ANY;
        sinInterface.sin_port = htons(uint16_t(par("tunnelPort")));
        if (bind(listenerSocket, (sockaddr*)&sinInterface, sizeof(sockaddr_in)) == SOCKET_ERROR)
            throw cRuntimeError("socket bind() failed");

        listen(listenerSocket, SOMAXCONN);

        rtScheduler->addSocket(this, gate("xx"), listenerSocket, true);
    }
}


void TCPClientTunnel::handleMessage(cMessage *msg)
{
}

bool TCPClientTunnel::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
}
