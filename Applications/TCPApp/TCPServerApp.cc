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
    port = par("port");

    serverSocket.bind(port);
    serverSocket.listen(true);
}

void TCPServerApp::handleMessage(cMessage *msg)
{
    TCPSocket *socket = socketMap.findSocketFor(msg);
    if (!socket)
    {
            socket = new TCPSocket(msg);
            socket->setOutputGate(gate("tcpOut"));

            TCPServerProcess *proc = NULL;//FIXME
            socket->setCallbackObject(proc);
            socketMap.addSocket(socket);
    }
    socket->processMessage(msg);
}

void TCPServerApp::finish()
{
}

/*
void TCPServerApp::socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);
void TCPServerApp::socketEstablished(int connId, void *yourPtr);
void TCPServerApp::socketPeerClosed(int connId, void *yourPtr);
void TCPServerApp::socketClosed(int connId, void *yourPtr);
void TCPServerApp::socketFailure(int connId, void *yourPtr, int code);
void TCPServerApp::socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
*/
