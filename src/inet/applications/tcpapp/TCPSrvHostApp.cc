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

#include "inet/applications/tcpapp/TCPSrvHostApp.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/INETUtils.h"

namespace inet {

Define_Module(TCPSrvHostApp);

void TCPSrvHostApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_APPLICATION_LAYER) {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");

        serverSocket.setOutputGate(gate("tcpOut"));
        serverSocket.readDataTransferModePar(*this);
        serverSocket.bind(localAddress[0] ? L3Address(localAddress) : L3Address(), localPort);
        serverSocket.listen();

        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void TCPSrvHostApp::updateDisplay()
{
    if (!hasGUI())
        return;

    char buf[32];
    sprintf(buf, "%d threads", socketMap.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void TCPSrvHostApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        TCPServerThreadBase *thread = (TCPServerThreadBase *)msg->getContextPointer();
        thread->timerExpired(msg);
    }
    else {
        TCPSocket *socket = socketMap.findSocketFor(msg);

        if (!socket) {
            // new connection -- create new socket object and server process
            socket = new TCPSocket(msg);
            socket->setOutputGate(gate("tcpOut"));

            const char *serverThreadClass = par("serverThreadClass");
            TCPServerThreadBase *proc =
                check_and_cast<TCPServerThreadBase *>(inet::utils::createOne(serverThreadClass));

            socket->setCallbackObject(proc);
            proc->init(this, socket);

            socketMap.addSocket(socket);

            updateDisplay();
        }

        socket->processMessage(msg);
    }
}

void TCPSrvHostApp::finish()
{
}

void TCPSrvHostApp::removeThread(TCPServerThreadBase *thread)
{
    // remove socket
    socketMap.removeSocket(thread->getSocket());

    // remove thread object
    delete thread;

    updateDisplay();
}

} // namespace inet

