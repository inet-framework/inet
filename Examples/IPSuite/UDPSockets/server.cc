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
class UDPSocketTestServer: public cSimpleModule
{
    Module_Class_Members(UDPSocketTestServer, cSimpleModule, 16384);
    virtual void activity();
};

Define_Module_Like( UDPSocketTestServer, SocketApp);

void UDPSocketTestServer::activity()
{
    SocketInterfacePacket* sockipack;
    Socket::Filedesc filedesc, filedesc_reply;
    SocketInterfacePacket::SockAction action;
    IPAddress remote_addr;
    PortNumber remote_port;
    cQueue garbage("garbage"); // FIXME check what it collects...

    //
    // SOCKET
    //
    sockipack = new SocketInterfacePacket("SocketCall");

    ev << "Calling function socket()\n";

    sockipack->socket(Socket::SOCKET_AF_INET, Socket::SOCKET_DGRAM, Socket::UDP);
    //sockipack->socket(Socket::AF_INET, Socket::SOCK_DGRAM, Socket::UDP);

    ev << "Sending message to Socketlayer\n";

    send(sockipack, "out");

    ev << "Waiting for message from Socketlayer\n";

    sockipack = (SocketInterfacePacket*) receive();

    filedesc = sockipack->filedesc();

    ev << "Received message from Socketlayer\n";
    ev << "Value of Filedescriptor: " << filedesc << endl;

    delete sockipack;

    // BIND

    sockipack = new SocketInterfacePacket("BindCall");

    ev <<"Calling function bind()\n";

    sockipack->bind(filedesc, IPADDRESS_UNDEF, 30);

    ev << "Sending message to Socketlayer\n";

    send(sockipack, "out");

    waitAndEnqueue(1, &garbage);

    //
    // READ
    //

    sockipack = (SocketInterfacePacket*) receive();

    cMessage* msg=sockipack->decapsulate();

    ev << "Message from Client: " << msg->par("Index") << " " << msg->par("Message") << endl;

    remote_addr = sockipack->fAddr();
    remote_port = sockipack->fPort();

    delete msg;
    delete sockipack;

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

    filedesc_reply = sockipack->filedesc();

    ev << "Received message from Socketlayer\n";
    ev << "Value of Filedescriptor: " << filedesc_reply << endl;

    delete sockipack;

    //
    // CONNECT
    //

    sockipack = new SocketInterfacePacket("ConnectCall");

    ev <<"Calling function connect()\n";

    sockipack->connect(filedesc_reply, remote_addr, remote_port);

    ev << "Sending message to Socketlayer\n";

    send(sockipack, "out");

    ev <<"Waiting for Socketlayer to establish connection\n";

    sockipack = (SocketInterfacePacket*) receive();

    action = sockipack->action();
    ev <<"Received message from Socketlayer\n";
    ev <<"Status of Client: " << action << endl;

    if (action == SocketInterfacePacket::SA_CONNECT_RET)
        ev << "Status of Client: SA_CONNECT_RET, Connection OK\n" ;
    else
        opp_error("Connection failed");

    delete sockipack;

    //
    // WRITE
    //

    cMessage* datapack = new cMessage("Payload");

    datapack->addPar("Message") = "A reply";
    sockipack = new SocketInterfacePacket("APPL-DATA");
    sockipack->write(filedesc_reply, datapack);
    send(sockipack, "out");
    waitAndEnqueue(20000, &garbage);

    //
    // CLOSE
    //

    sockipack = new SocketInterfacePacket("CloseCall");

    ev <<"Calling function close() on filedesc " << (int) filedesc << endl;

    sockipack->close(filedesc);
    send(sockipack, "out");

    sockipack = new SocketInterfacePacket("CloseCall");

    ev <<"Calling function close() on filedesc " << (int) filedesc_reply << endl;

    sockipack->close(filedesc_reply);
    send(sockipack, "out");

    ev <<"Connection is closed\n";
}
