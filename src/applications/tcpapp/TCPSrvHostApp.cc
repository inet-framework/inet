//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


#include "TCPSrvHostApp.h"


Define_Module(TCPSrvHostApp);


void TCPSrvHostApp::initialize()
{
    cSimpleModule::initialize();

    const char *localAddress = par("localAddress");
    int localPort = par("localPort");

    serverSocket.setOutputGate(gate("tcpOut"));
    serverSocket.readDataTransferModePar(*this);
    serverSocket.bind(localAddress[0] ? IPvXAddress(localAddress) : IPvXAddress(), localPort);
    serverSocket.listen();
}

void TCPSrvHostApp::updateDisplay()
{
    if (!ev.isGUI())
        return;

    char buf[32];
    sprintf(buf, "%d threads", socketMap.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void TCPSrvHostApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        TCPServerThreadBase *thread = (TCPServerThreadBase *)msg->getContextPointer();
        thread->timerExpired(msg);
    }
    else
    {
        TCPSocket *socket = socketMap.findSocketFor(msg);

        if (!socket)
        {
            // new connection -- create new socket object and server process
            socket = new TCPSocket(msg);
            socket->setOutputGate(gate("tcpOut"));

            const char *serverThreadClass = par("serverThreadClass");
            TCPServerThreadBase *proc =
                    check_and_cast<TCPServerThreadBase *>(createOne(serverThreadClass));

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

