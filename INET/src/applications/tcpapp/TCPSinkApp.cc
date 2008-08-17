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


#include "TCPSinkApp.h"
#include "TCPSocket.h"
#include "TCPCommand_m.h"


Define_Module(TCPSinkApp);

void TCPSinkApp::initialize()
{
    const char *address = par("address");
    int port = par("port");

    bytesRcvd = 0;
    WATCH(bytesRcvd);

    TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    socket.bind(address[0] ? IPvXAddress(address) : IPvXAddress(), port);
    socket.listen();
}

void TCPSinkApp::handleMessage(cMessage *msg)
{
    if (msg->getKind()==TCP_I_PEER_CLOSED)
    {
        // we close too
        msg->setKind(TCP_C_CLOSE);
        send(msg, "tcpOut");
    }
    else if (msg->getKind()==TCP_I_DATA || msg->getKind()==TCP_I_URGENT_DATA)
    {
        bytesRcvd += PK(msg)->getByteLength();
        delete msg;

        if (ev.isGUI())
        {
            char buf[32];
            sprintf(buf, "rcvd: %ld bytes", bytesRcvd);
            getDisplayString().setTagArg("t",0,buf);
        }
    }
    else
    {
        // must be data or some kind of indication -- can be dropped
        delete msg;
    }
}

void TCPSinkApp::finish()
{
    recordScalar("bytesRcvd", bytesRcvd);
}

