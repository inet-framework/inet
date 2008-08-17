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

TelnetApp::TelnetApp()
{
    timeoutMsg = NULL;
}

TelnetApp::~TelnetApp()
{
    cancelAndDelete(timeoutMsg);
}

void TelnetApp::initialize()
{
    TCPGenericCliAppBase::initialize();

    timeoutMsg = new cMessage("timer");

    numCharsToType = numLinesToType = 0;
    WATCH(numCharsToType);
    WATCH(numLinesToType);

    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt((simtime_t)par("startTime"), timeoutMsg);
}

void TelnetApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind())
    {
        case MSGKIND_CONNECT:
            EV << "user fires up telnet program\n";
            connect();
            break;

        case MSGKIND_SEND:
           if (numCharsToType>0)
           {
               // user types a character and expects it to be echoed
               EV << "user types one character, " << numCharsToType-1 << " more to go\n";
               sendPacket(1,1);
               scheduleAt(simTime()+(simtime_t)par("keyPressDelay"), timeoutMsg);
               numCharsToType--;
           }
           else
           {
               EV << "user hits Enter key\n";
               // Note: reply length must be at least 2, otherwise we'll think
               // it's an echo when it comes back!
               sendPacket(1, 2+(long)par("commandOutputLength"));
               numCharsToType = (long)par("commandLength");

               // Note: no scheduleAt(), because user only starts typing next command
               // when output from previous one has arrived (see socketDataArrived())
           }
           break;

        case MSGKIND_CLOSE:
           EV << "user exits telnet program\n";
           close();
           break;
    }
}

void TelnetApp::socketEstablished(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketEstablished(connId, ptr);

    // schedule first sending
    numLinesToType = (long) par("numCommands");
    numCharsToType = (long) par("commandLength");
    timeoutMsg->setKind(numLinesToType>0 ? MSGKIND_SEND : MSGKIND_CLOSE);
    scheduleAt(simTime()+(simtime_t)par("thinkTime"), timeoutMsg);
}

void TelnetApp::socketDataArrived(int connId, void *ptr, cPacket *msg, bool urgent)
{
    int len = msg->getByteLength();
    TCPGenericCliAppBase::socketDataArrived(connId, ptr, msg, urgent);

    if (len==1)
    {
        // this is an echo, ignore
        EV << "received echo\n";
    }
    else
    {
        // output from last typed command arrived.
        EV << "received output of command typed\n";

        // If user has finished working, she closes the connection, otherwise
        // starts typing again after a delay
        numLinesToType--;

        if (numLinesToType==0)
        {
            EV << "user has no more commands to type\n";
            timeoutMsg->setKind(MSGKIND_CLOSE);
            scheduleAt(simTime()+(simtime_t)par("thinkTime"), timeoutMsg);
        }
        else
        {
            EV << "user looks at output, then starts typing next command\n";
            timeoutMsg->setKind(MSGKIND_SEND);
            scheduleAt(simTime()+(simtime_t)par("thinkTime"), timeoutMsg);
        }
    }
}

void TelnetApp::socketClosed(int connId, void *ptr)
{
    TCPGenericCliAppBase::socketClosed(connId, ptr);

    // start another session after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+(simtime_t)par("idleInterval"), timeoutMsg);
}

void TelnetApp::socketFailure(int connId, void *ptr, int code)
{
    TCPGenericCliAppBase::socketFailure(connId, ptr, code);

    // reconnect after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime()+(simtime_t)par("reconnectInterval"), timeoutMsg);
}

