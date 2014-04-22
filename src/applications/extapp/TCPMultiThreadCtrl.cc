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

#include "TCPMultiThreadCtrl.h"

#include "AddressResolver.h"
#include "ModuleAccess.h"
#include "NodeStatus.h"

//Define_Module(TCPMultiThreadCtrl);

void TCPThreadBase::connect(Address destination, int connectPort)
{
    EV_INFO << "Connecting to " << destination << " port=" << connectPort << endl;
    socket.renewSocket();
    socket.connect(destination, connectPort);
}

void TCPThreadBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        socket.setCallbackObject(this);
        socket.setOutputGate(gate("tcpOut"));
        socket.readDataTransferModePar(*this);
    }
}

void TCPThreadBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else
        socket.processMessage(msg);
}

void TCPMultiThreadCtrl::initialize(int stage)
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

TCPThreadBase *TCPMultiThreadCtrl::findThreadFor(cMessage *msg)
{
    TCPCommand *ind = dynamic_cast<TCPCommand *>(msg->getControlInfo());
    if (!ind)
        throw cRuntimeError("TCPMultiThreadCtrl: findThreadFor: no TCPCommand control info in message (not from TCP?)");

    int connId = ind->getConnId();
    TCPThreadMap::iterator i = threadMap.find(connId);
    ASSERT(i==threadMap.end() || i->first==i->second->getSocket()->getConnectionId());
    return (i==threadMap.end()) ? NULL : i->second;
}


void TCPMultiThreadCtrl::updateDisplay()
{
    if (!ev.isGUI())
        return;

    char buf[32];
    sprintf(buf, "%d threads", (int)threadMap.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void TCPMultiThreadCtrl::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        handleSelfMessage(msg);
    }
    else
    {
        TCPThreadBase *thread = findThreadFor(msg);
        if (!thread)
        {
            thread = createNewThreadFor(msg);
            updateDisplay();
        }

        if (thread)
            thread->getSocket()->processMessage(msg);
        else
        {
            EV_WARN << "message dropped because thread not found for it\n";
            delete msg;
        }
    }
}

TCPThreadBase *TCPMultiThreadCtrl::createNewThreadFor(cMessage *msg)
{
    Enter_Method_Silent();

    // new connection -- create new socket object and server process
    const char *moduleNedType = par("threadClass");
    cModuleType *moduleType = cModuleType::find(moduleNedType);
    if (moduleType == NULL)
        throw cRuntimeError("TCPMultiThreadCtrl: module '%s' not found", moduleNedType);

    TCPThreadBase *thread = check_and_cast<TCPThreadBase *>(moduleType->create(moduleNedType, this));
    thread->finalizeParameters();
    thread->callInitialize();
    char moduleName[100];
    sprintf(moduleName, "connect-%i", thread->getSocket()->getConnectionId());
    thread->setName(moduleName);
    threadMap.insert(std::pair<int, TCPThreadBase *>(thread->getSocket()->getConnectionId(), thread));
    return thread;
}

void TCPMultiThreadCtrl::finish()
{
}

void TCPMultiThreadCtrl::removeThread(TCPThreadBase *thread)
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

bool TCPMultiThreadCtrl::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
}

