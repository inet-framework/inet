//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <omnetpp.h>
#include "SocketInterfacePacket.h"

/**
 * For testing the UDP socket layer
 */
class UDPSocketTestClient: public cSimpleModule
{
    Module_Class_Members(UDPSocketTestClient, cSimpleModule, 16384);
    virtual void activity();
};

Define_Module_Like( UDPSocketTestClient, SocketApp);

void UDPSocketTestClient::activity()
{
    SocketInterfacePacket* sockipack = NULL;
    cMessage* datapack = NULL;
    Socket::Filedesc filedesc;
    SocketInterfacePacket::SockAction action;
    cQueue garbage("garbage"); // FIXME check what it collects...

    // wait till server is in listen state
    waitAndEnqueue(10, &garbage);

    //
    // SOCKET
    //

    sockipack = new SocketInterfacePacket("SocketCall");

    ev << "Calling function socket()\n";

    sockipack->socket(Socket::SOCKET_AF_INET, Socket::SOCKET_DGRAM, Socket::UDP);

    ev << "Sending message to Socketlayer\n";
    send(sockipack, "out");

    ev << "Waiting for message from Socketlayer\n";

    sockipack = (SocketInterfacePacket*) receive();

    filedesc = sockipack->filedesc();

    ev << "Received message from Socketlayer\n";
    ev << "Value of Filedescriptor: " << filedesc << endl;

    delete sockipack;

    //
    // CONNECT
    //

    sockipack = new SocketInterfacePacket("ConnectCall");

    ev <<"Calling function connect()\n";
    sockipack->connect(filedesc, "10.0.0.1", 30);

    ev << "Sending message to Socketlayer\n";
    send(sockipack, "out");
    ev <<"Waiting for Socketlayer to establish connection\n";
    sockipack = (SocketInterfacePacket*) receive();

    action = sockipack->action();
    ev <<"Received message from Socketlayer\n";
    ev <<"Status of Client: " << action << endl;

    if (action == SocketInterfacePacket::SA_CONNECT_RET)
        ev <<"Status of Client: SA_CONNECT_RET, Connection OK\n" ;
    else
        error ("Connection failed");

    delete sockipack;

    //
    // WRITE
    //

    sockipack = new SocketInterfacePacket("WriteCall");
    datapack = new cMessage("ApplData");
    datapack->addPar("Message") = "Hallo Du Da";

    datapack->addPar("Index") = idx;

    ev <<"Calling function write()\n";

    sockipack->write(filedesc, datapack);

    ev << "Sending message to Socketlayer\n";

    send(sockipack, "out");
    waitAndEnqueue(1, &garbage);

    //
    // READ
    //

    sockipack = (SocketInterfacePacket*) receive();

    datapack = sockipack->decapsulate();

    ev << "Message received from Server: " << datapack->par("Message") << endl;

    delete datapack;
    delete sockipack;

    //
    // CLOSE
    //

    wait (200);

    sockipack = new SocketInterfacePacket("CloseCall");

    ev <<"Calling function close()\n";

    sockipack->close(filedesc);
    send(sockipack, "out");
    ev <<"Connection is closed\n";
}
