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


#include "TCPEchoApp.h"
#include "TCPSocket.h"
#include "TCPCommand_m.h"


Define_Module(TCPEchoApp);

void TCPEchoApp::initialize()
{
    const char *address = par("address");
    int port = par("port");
    double delay = par("replyDelay");

    bytesRcvd = bytesSent = 0;
    WATCH(bytesRcvd);
    WATCH(bytesSent);

    TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    socket.bind(address[0] ? IPAddress(address) : IPAddress(), port);
    socket.listen(true);
}

void TCPEchoApp::sendOrSchedule(cMessage *msg)
{
    if (delay==0)
    {
        bytesSent += msg->length()/8;
        send(msg, "tcpOut");
    }
    else
    {
        scheduleAt(simTime()+delay, msg);
    }
}

void TCPEchoApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        bytesSent += msg->length()/8;
        send(msg, "tcpOut");
    }
    else if (msg->kind()==TCP_I_PEER_CLOSED)
    {
        // we'll close too
        msg->setKind(TCP_C_CLOSE);
        sendOrSchedule(msg);
    }
    else if (msg->kind()==TCP_I_DATA || msg->kind()==TCP_I_URGENT_DATA)
    {
        bytesRcvd += msg->length()/8;
        msg->setKind(TCP_C_SEND);
        long len = par("replyLength");
        if (len>0)
            msg->setLength(8*len);
        sendOrSchedule(msg);
    }
    else
    {
        // some indication -- ignore
        delete msg;
    }

    if (ev.isGUI())
    {
        char buf[32];
        sprintf(buf, "rcvd: %ld bytes\nsent: %ld bytes", bytesRcvd, bytesSent);
        displayString().setTagArg("t",0,buf);
    }
}

void TCPEchoApp::finish()
{
}

