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

void TCPGenericCliAppBase::count(cMessage *msg)
{
    if (msg->kind()==TCP_I_DATA || msg->kind()==TCP_I_URGENT_DATA)
    {
        packetsRcvd++;
        bytesRcvd+=msg->length()/8;
    }
    else
    {
        indicationsRcvd++;
    }
}

void TCPGenericCliAppBase::waitUntil(simtime_t t)
{
    if (simTime()>=t)
        return;

    cMessage *timeoutMsg = new cMessage("timeout");
    scheduleAt(t, timeoutMsg);
    cMessage *msg=NULL;
    while ((msg=receive())!=timeoutMsg)
    {
        count(msg);
        socket.processMessage(msg);
    }
    delete timeoutMsg;
}

void TCPGenericCliAppBase::activity()
{
    packetsRcvd = bytesRcvd = indicationsRcvd = 0;
    WATCH(packetsRcvd);
    WATCH(bytesRcvd);
    WATCH(indicationsRcvd);

    // parameters
    const char *address = par("address");
    int port = par("port");
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");

    bool active = par("active");
    simtime_t tOpen = par("tOpen");
    simtime_t tSend = par("tSend");
    simtime_t sendBytes = par("sendBytes");
    simtime_t tClose = par("tClose");

    socket.setOutputGate(gate("tcpOut"));

    // open
    waitUntil(tOpen);

    if (!address[0])
        socket.bind(port);
    else
        socket.bind(IPAddress(address), port);

    ev << "issuing OPEN command\n";
    if (ev.isGUI()) displayString().setTagArg("t",0, active?"connecting":"listening");

    if (active)
        socket.connect(IPAddressResolver().resolve(connectAddress), connectPort);
    else
        socket.listen();

    // wait until connection gets established
    while (socket.state()!=TCPSocket::CONNECTED)
    {
        socket.processMessage(receive());
        if (socket.state()==TCPSocket::SOCKERROR)
            return;
    }

    ev << "connection established, starting sending\n";
    if (ev.isGUI()) displayString().setTagArg("t",0,"connected");

    // send
    if (sendBytes>0)
    {
        waitUntil(tSend);
        ev << "sending " << sendBytes << " bytes\n";
        cMessage *msg = new cMessage("data1");
        msg->setLength(8*sendBytes);
        socket.send(msg);
    }

    // close
    if (tClose>=0)
    {
        waitUntil(tClose);
        ev << "issuing CLOSE command\n";
        if (ev.isGUI()) displayString().setTagArg("t",0,"closing");
        socket.close();
    }

    // wait until peer closes too and all data arrive
    for (;;)
    {
        cMessage *msg = receive();
        count(msg);
        socket.processMessage(msg);
    }
}

void TCPGenericCliAppBase::finish()
{
    ev << fullPath() << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
}


