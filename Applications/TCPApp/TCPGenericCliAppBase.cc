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


#include "TCPGenericCliAppBase.h"
#include "IPAddressResolver.h"


Define_Module(TCPGenericCliAppBase);


void TCPGenericCliAppBase::initialize()
{
    packetsRcvd = bytesRcvd = 0;
    WATCH(packetsRcvd);
    WATCH(bytesRcvd);

    // parameters
    const char *address = par("address");
    int port = par("port");
    if (!address[0])
        socket.bind(port);
    else
        socket.bind(IPAddress(address), port);

    socket.setCallbackObject(this);
    socket.setOutputGate(gate("tcpOut"));

    if (ev.isGUI()) displayString().setTagArg("t",0, "waiting");

    simtime_t t = getConnectTime(msg);
    cMessage *timerMsg = new cMessage("timer", TIMER_CONNECT);
    scheduleAt(t, timerMsg);
}

void TCPGenericCliAppBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else
        socket.processMessage(msg);
}

void TCPGenericCliAppBase::handleTimer(cMessage *msg)
{
    switch (msg->kind())
    {
        case TIMER_CONNECT: connect(); break;
        //...
    }
}

void TCPGenericCliAppBase::connect()
{
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");

    ev << "issuing OPEN command\n";
    if (ev.isGUI()) displayString().setTagArg("t",0, active?"connecting");

    socket.connect(IPAddressResolver().resolve(connectAddress), connectPort);
}

void TCPGenericCliAppBase::socketEstablished(int connId, void *yourPtr)
{
    // do first sending, or at least schedule it
}

void TCPGenericCliAppBase::socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent)
{
        packetsRcvd++;
        bytesRcvd+=msg->length()/8;
}

void TCPGenericCliAppBase::socketPeerClosed(int connId, void *yourPtr)
{
}

void TCPGenericCliAppBase::socketClosed(int connId, void *yourPtr)
{
}

void TCPGenericCliAppBase::socketFailure(int connId, void *yourPtr, int code)
{
}

void TCPGenericCliAppBase::finish()
{
    ev << fullPath() << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
}


