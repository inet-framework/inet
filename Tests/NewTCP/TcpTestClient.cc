//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Copyright 2004 Andras Varga
//

#include <omnetpp.h>
#include "TCPSocket.h"


class TcpTestClient : public cSimpleModule
{
  public:
    cQueue queue;

    Module_Class_Members(TcpTestClient, cSimpleModule, 16384);
    virtual void activity();
    virtual void finish();
};

Define_Module(TcpTestClient);

void TcpTestClient::activity()
{
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
    TCPSocket socket;
    queue.setName("queue");

    // open
    waitAndEnqueue(tOpen-simTime(), &queue);

    socket.bind(IPAddress(address), port);

    if (active)
        socket.connect(IPAddress(connectAddress), connectPort);
    else
        socket.accept();

    // send
    if (sendBytes>0)
    {
        waitAndEnqueue(tSend-simTime(), &queue);

        cMessage *msg = new cMessage("data1");
        msg->setLength(8*sendBytes);
        socket.send(msg);
    }

    // close
    if (tClose>=0)
    {
        waitAndEnqueue(tClose-simTime(), &queue);
        socket.close();
    }

    while (true)
    {
        cMessage *msg = receive();
        queue.insert(msg);
    }
}

void TcpTestClient::finish()
{
    while (!queue.empty())
    {
        cMessage *msg = (cMessage *)queue.pop();
        ev << fullPath() << ": received " << msg->name() << ", " << msg->length()/8 << " bytes\n";
        delete msg;
    }
}
