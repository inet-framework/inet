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


#include "TelnetApp.h"


#define MSGKIND_CONNECT  0
#define MSGKIND_SEND     1
#define MSGKIND_CLOSE    2


void TelnetApp::initialize()
{
    TCPGenericCliAppBase::initialize();

    timeoutMsg = new cMessage("timer");
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(10, timeoutMsg);
}

void TelnetApp::handleTimer(cMessage *msg)
{
    switch (msg->kind())
    {
        case MSGKIND_CONNECT:
            connect();
            break;
        case MSGKIND_SEND:
           sendPacket(1,1); // type a character and expect to be echoed
           scheduleAt(simTime()+10, timeoutMsg);
           break;
        case MSGKIND_CLOSE:
           close();
           break;
    }
}

void TelnetApp::socketEstablished(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketEstablished(connId, ptr);

    timeoutMsg->setKind(MSGKIND_SEND);
    scheduleAt(simTime()+10, timeoutMsg);
}

void TelnetApp::socketDataArrived(int connId, void *ptr, cMessage *msg, bool urgent)
{
    TCPGenericCliAppBase::socketDataArrived(connId, ptr, msg, urgent);

    timeoutMsg->setKind(MSGKIND_SEND);
    scheduleAt(simTime()+10, timeoutMsg);
}

void TelnetApp::socketClosed(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketClosed(connId, ptr);

    // start another session after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+10, timeoutMsg);
}

void TelnetApp::socketFailure(int connId, void *ptr, int code)
{
    TCPGenericCliAppBase::socketFailure(connId, ptr, code);

    // reconnect after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+10, timeoutMsg);
}

