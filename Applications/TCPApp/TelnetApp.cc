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


Define_Module(TelnetApp);


void TelnetApp::initialize()
{
    TCPGenericCliAppBase::initialize();

    timeoutMsg = new cMessage("timer");

    numCharsToType = numLinesToType = 0;
    WATCH(numCharsToType);
    WATCH(numLinesToType);

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
           if (numCharsToType>1)
           {
               // user types a character and expects it to be echoed
               ev << "user types one character, " << numCharsToType-1 << " to go\n";
               sendPacket(1,1);
               scheduleAt(simTime()+0.1, timeoutMsg);
               numCharsToType--;
           }
           else
           {
               ev << "user hits Enter key\n";
               sendPacket(1,50); // user hits Enter, and waits for type command's output
               numCharsToType = 15;

               // Note: no scheduleAt(), because user only starts typing next command
               // when output from previous one has arrived (see socketDataArrived())
           }
           break;

        case MSGKIND_CLOSE:
           close();
           break;
    }
}

void TelnetApp::socketEstablished(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketEstablished(connId, ptr);

    // schedule first sending
    numLinesToType = 5;
    numCharsToType = 15;
    timeoutMsg->setKind(MSGKIND_SEND);
    scheduleAt(simTime()+1, timeoutMsg);
}

void TelnetApp::socketDataArrived(int connId, void *ptr, cMessage *msg, bool urgent)
{
    int len = msg->length()/8;
    TCPGenericCliAppBase::socketDataArrived(connId, ptr, msg, urgent);

    if (len==1)
    {
        // this is an echo, ignore
        ev << "received echo\n";
    }
    else
    {
        // output from last typed command arrived.
        ev << "received output of last command typed\n";

        // If user has finished working, she closes the connection, otherwise
        // starts typing again after a delay
        numLinesToType--;

        if (numLinesToType==0)
        {
            ev << "user finished session, closing TCP connection\n";
            close();
        }
        else
        {
            ev << "user looks at output, then starts typing next command\n";
            timeoutMsg->setKind(MSGKIND_SEND);
            scheduleAt(simTime()+3, timeoutMsg);
        }
    }
}

void TelnetApp::socketClosed(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketClosed(connId, ptr);

    // start another session after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+600, timeoutMsg);
}

void TelnetApp::socketFailure(int connId, void *ptr, int code)
{
    TCPGenericCliAppBase::socketFailure(connId, ptr, code);

    // reconnect after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+30, timeoutMsg);
}

