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


#include "TCPServerApp.h"


Define_Module(TCPServerApp);

void TCPServerApp::initialize()
{
    int port = par("port");

    const char *serverProcTypeName = par("serverProcess");
    srvProcType = findModuleType(serverProcTypeName);
    if (!srvProcType)
        error("module type serverProcess=`%s' not found", serverProcTypeName);

    serverSocket.setOutputGate(gate("tcpOut"));
    serverSocket.bind(port);
    serverSocket.listen(true);
}

void TCPServerApp::handleMessage(cMessage *msg)
{
    TCPSocket *socket = socketMap.findSocketFor(msg);
    if (!socket)
    {
        // new connection -- create new socket object and server process
        socket = new TCPSocket(msg);
        socket->setOutputGate(gate("tcpOut"));

        cModule *mod = srvProcType->createScheduleInit("serverproc",this);
        TCPServerProcess *proc = check_and_cast<TCPServerProcess *>(mod);
        socket->setCallbackObject(proc);
        proc->setSocket(socket);

        socketMap.addSocket(socket);
    }
    socket->processMessage(msg);
}

void TCPServerApp::finish()
{
}

//---

void TCPServerProcess::removeSocket()
{
    // FIXME TBD
}

