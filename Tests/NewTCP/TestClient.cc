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
    Module_Class_Members(TcpTestClient, cSimpleModule, 16384);
    virtual void activity();
};

Define_Module(TcpTestClient);

void TcpTestClient::activity()
{
    cMessage *msg;
    TCPSocket socket;
    cQueue queue("queue");

    if ((bool)par("active"))
    {
        waitAndEnqueue(1, &queue);

        socket.bind(IPAddress("10.0.0.1"),1000);
        socket.connect(IPAddress("10.0.0.2"),2000);

        waitAndEnqueue(1, &queue);

        msg = new cMessage("data1");
        msg->setLength(8*16*1024);  // 16K
        socket.send(msg);

        //waitAndEnqueue(10, &queue);

        socket.close();
    }
    else
    {
        //socket.bind(IPAddress("10.0.0.2"),2000);
        socket.bind(2000);
        socket.accept();

        waitAndEnqueue(20, &queue);

        socket.close();
    }

    while (true)
    {
        cMessage *msg = queue.empty() ? receive() : (cMessage *)queue.pop();
        ev << "Received " << msg->name() << ", " << msg->length()/8 << " bytes\n";
        //delete msg; -- preserve them for inspection
    }
}


