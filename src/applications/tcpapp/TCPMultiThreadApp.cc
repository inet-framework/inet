//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "TCPMultiThreadApp.h"

#include "AddressResolver.h"
#include "ModuleAccess.h"
#include "NodeStatus.h"

Define_Module(TCPMultiThreadApp);


void TCPMultiThreadApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

TCPThreadBase *TCPMultiThreadApp::findThreadFor(cMessage *msg)
{
    TCPCommand *ind = dynamic_cast<TCPCommand *>(msg->getControlInfo());
    if (!ind)
        throw cRuntimeError("TCPMultiThreadApp: findThreadFor: no TCPCommand control info in message (not from TCP?)");

    int connId = ind->getConnId();
    TCPThreadMap::iterator i = threadMap.find(connId);
    ASSERT(i==threadMap.end() || i->first==i->second->getSocket()->getConnectionId());
    return (i==threadMap.end()) ? NULL : i->second;
}


void TCPMultiThreadApp::updateDisplay()
{
    if (!ev.isGUI())
        return;

    char buf[32];
    sprintf(buf, "%d threads", (int)threadMap.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void TCPMultiThreadApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        TCPThreadBase *thread = (TCPThreadBase *)msg->getContextPointer();
        thread->timerExpired(msg);
    }
    else
    {
        TCPThreadBase *thread = findThreadFor(msg);

        if (!thread)
        {
            // new connection -- create new socket object and server process
            TCPSocket *socket = new TCPSocket(msg);
            socket->setOutputGate(gate("tcpOut"));
            const char *serverThreadClass = par("serverThreadClass");
            thread = check_and_cast<TCPThreadBase *>(createOne(serverThreadClass));
            thread->finalizeParameters();
            socket->setCallbackObject(thread);
            thread->init(this, socket);
            thread->callInitialize();
            threadMap.insert(std::pair<int, TCPThreadBase *>(socket->getConnectionId(), thread));

            updateDisplay();
        }

        thread->getSocket()->processMessage(msg);
    }
}

void TCPMultiThreadApp::finish()
{
    for (TCPThreadMap::iterator i = threadMap.begin(); i != threadMap.end(); ++i)
        i->second->callFinish();
}

void TCPMultiThreadApp::removeThread(TCPThreadBase *thread)
{
    TCPThreadMap::iterator i = threadMap.find(thread->getSocket()->getConnectionId());
    if (i != threadMap.end())
    {
        i->second->callFinish();
        threadMap.erase(i);
    }

    // remove thread object
    delete thread;

    updateDisplay();
}

bool TCPMultiThreadApp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
}

