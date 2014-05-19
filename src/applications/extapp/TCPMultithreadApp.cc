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

#include "TCPMultithreadApp.h"

#include "AddressResolver.h"
#include "ModuleAccess.h"
#include "NodeStatus.h"

Define_Module(TCPMultithreadApp);


void TCPMultithreadApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(getContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        mainThread = check_and_cast<ITCPAppThread *>(createOne(par("mainThreadClass")));
        mainThread->init(this, gate("tcpOut"), NULL);
    }
}

void TCPMultithreadApp::updateDisplay()
{
    if (!ev.isGUI())
        return;

    char buf[32];
    sprintf(buf, "%d threads", threadMap.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void TCPMultithreadApp::handleMessage(cMessage *msg)
{
    ITCPAppThread *thread = threadMap.findThreadFor(msg);
    if (!thread)
    {
        // new connection -- create new thread
        const char *activeThreadClass = par("activeThreadClass");
        thread = check_and_cast<ITCPAppThread *>(createOne(activeThreadClass));
        thread->init(this, gate("tcpOut"), msg);
        threadMap.addThread(thread);

        updateDisplay();
    }

    thread->processMessage(msg);
}

void TCPMultithreadApp::finish()
{
}

void TCPMultithreadApp::addThread(ITCPAppThread *thread)
{
    threadMap.addThread(thread);

    updateDisplay();
}

void TCPMultithreadApp::removeThread(ITCPAppThread *thread)
{
    // remove socket
    threadMap.removeThread(thread);

    // remove thread object
    delete thread;

    updateDisplay();
}

bool TCPMultithreadApp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}
