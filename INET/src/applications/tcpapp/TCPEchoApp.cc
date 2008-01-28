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
    delay = par("echoDelay");
    echoFactor = par("echoFactor");

    bytesRcvd = bytesSent = 0;
    WATCH(bytesRcvd);
    WATCH(bytesSent);

    TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    socket.bind(address[0] ? IPvXAddress(address) : IPvXAddress(), port);
    socket.listen();
}

void TCPEchoApp::sendOrSchedule(cMessage *msg)
{
    if (delay==0)
    {
        bytesSent += msg->byteLength();
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
        bytesSent += msg->byteLength();
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
        bytesRcvd += msg->byteLength();
        if (echoFactor==0)
        {
            delete msg;
        }
        else
        {
            // reverse direction, modify length, and send it back
            msg->setKind(TCP_C_SEND);
            TCPCommand *ind = check_and_cast<TCPCommand *>(msg->removeControlInfo());
            TCPSendCommand *cmd = new TCPSendCommand();
            cmd->setConnId(ind->connId());
            msg->setControlInfo(cmd);
            delete ind;

            long byteLen = msg->byteLength()*echoFactor;
            if (byteLen<1) byteLen=1;
            msg->setByteLength(byteLen);

            sendOrSchedule(msg);
        }
    }
    else
    {
        // some indication -- ignore
        delete msg;
    }

    if (ev.isGUI())
    {
        char buf[80];
        sprintf(buf, "rcvd: %ld bytes\nsent: %ld bytes", bytesRcvd, bytesSent);
        displayString().setTagArg("t",0,buf);
    }
}

void TCPEchoApp::finish()
{
}

